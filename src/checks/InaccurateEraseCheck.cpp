//===--- InaccurateEraseCheck.cpp - clang-tidy-----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "InaccurateEraseCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {

void InaccurateEraseCheck::registerMatchers(MatchFinder *Finder) {
  const auto EndCall =
      callExpr(
          callee(functionDecl(hasAnyName("remove", "remove_if", "unique"))),
          hasArgument(
              1, optionally(cxxMemberCallExpr(callee(cxxMethodDecl(hasName("end")))))));

  const auto DeclInStd = type(hasUnqualifiedDesugaredType(
      tagType(hasDeclaration(decl(isInStdNamespace())))));
  Finder->addMatcher(
      cxxMemberCallExpr(
          on(anyOf(hasType(DeclInStd), hasType(pointsTo(DeclInStd)))),
          callee(cxxMethodDecl(hasName("erase"))), argumentCountIs(1),
          hasArgument(0, EndCall))
          .bind("erase"),
      this);
}

void InaccurateEraseCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *MemberCall =
      Result.Nodes.getNodeAs<CXXMemberCallExpr>("erase");

  std::string msg = "Insufficient deletion of removed elements using `" + GetStrRepr(
      MemberCall->getMethodDecl(), *Result.SourceManager) + "`";

  registerIssue(msg, MemberCall, *Result.SourceManager); 
}

} // namespace tidy
} // namespace clang
