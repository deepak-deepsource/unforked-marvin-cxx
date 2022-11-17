//===--- CopyConstructorInitCheck.cpp - clang-tidy-------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CopyConstructorInitCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {

void CopyConstructorInitCheck::registerMatchers(MatchFinder *Finder) {
  // In the future this might be extended to move constructors?
  Finder->addMatcher(
      cxxConstructorDecl(
          isCopyConstructor(),
          hasAnyConstructorInitializer(cxxCtorInitializer(
              isBaseInitializer(),
              withInitializer(cxxConstructExpr(hasDeclaration(
                  cxxConstructorDecl(isDefaultConstructor())))))),
          unless(isInstantiated()))
          .bind("ctor"),
      this);
}

void CopyConstructorInitCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
  std::string ParamName = Ctor->getParamDecl(0)->getNameAsString();

  // We want only one warning (and FixIt) for each ctor.
  std::string FixItInitList;
  bool HasRelevantBaseInit = false;
  bool ShouldNotDoFixit = false;
  bool HasWrittenInitializer = false;
  SmallVector<FixItHint, 2> SafeFixIts;
  for (const auto *Init : Ctor->inits()) {
    bool CtorInitIsWritten = Init->isWritten();
    HasWrittenInitializer = HasWrittenInitializer || CtorInitIsWritten;
    if (!Init->isBaseInitializer())
      continue;
    const Type *BaseType = Init->getBaseClass();
    // Do not do fixits if there is a type alias involved or one of the bases
    // are explicitly initialized. In the latter case we not do fixits to avoid
    // -Wreorder warnings.
    if (const auto *TempSpecTy = dyn_cast<TemplateSpecializationType>(BaseType))
      ShouldNotDoFixit = ShouldNotDoFixit || TempSpecTy->isTypeAlias();
    ShouldNotDoFixit = ShouldNotDoFixit || isa<TypedefType>(BaseType);
    ShouldNotDoFixit = ShouldNotDoFixit || CtorInitIsWritten;
    const CXXRecordDecl *BaseClass =
        BaseType->getAsCXXRecordDecl()->getDefinition();
    if (BaseClass->field_empty() &&
        BaseClass->forallBases(
            [](const CXXRecordDecl *Class) { return Class->field_empty(); }))
      continue;
    bool NonCopyableBase = false;
    for (const auto *Ctor : BaseClass->ctors()) {
      if (Ctor->isCopyConstructor() &&
          (Ctor->getAccess() == AS_private || Ctor->isDeleted())) {
        NonCopyableBase = true;
        break;
      }
    }
    if (NonCopyableBase)
      continue;
    const auto *CExpr = dyn_cast<CXXConstructExpr>(Init->getInit());
    if (!CExpr || !CExpr->getConstructor()->isDefaultConstructor())
      continue;
    HasRelevantBaseInit = true;
    if (CtorInitIsWritten) {
      if (!ParamName.empty())
        SafeFixIts.push_back(
            FixItHint::CreateInsertion(CExpr->getEndLoc(), ParamName));
    } else {
      if (Init->getSourceLocation().isMacroID() ||
          Ctor->getLocation().isMacroID() || ShouldNotDoFixit)
        break;
      FixItInitList += BaseClass->getNameAsString();
      FixItInitList += "(" + ParamName + "), ";
    }
  }
  if (!HasRelevantBaseInit)
    return;

  auto &SM = Result.Context->getSourceManager();
  std::string msg = "`" +
      GetStrRepr(Ctor, SM) +
      "` is calling a base constructor other than the copy constructor";
  registerIssue(msg, Ctor, SM);
}

} // namespace tidy
} // namespace clang
