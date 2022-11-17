//===--- BoolPointerImplicitConversionCheck.cpp - clang-tidy --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// // See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "BoolPointerImplicitConversionCheck.h"

using namespace clang::ast_matchers;
using namespace clang::marvincxx;

namespace clang {
namespace tidy {

void BoolPointerImplicitConversionCheck::registerMatchers(MatchFinder *Finder) {
  // Look for ifs that have an implicit bool* to bool conversion in the
  // condition. Filter negations.
  Finder->addMatcher(
      traverse(
          TK_AsIs,
          ifStmt(
              hasCondition(findAll(implicitCastExpr(
                  unless(hasParent(unaryOperator(hasOperatorName("!")))),
                  hasSourceExpression(expr(
                      hasType(pointerType(pointee(booleanType()))),
                      ignoringParenImpCasts(anyOf(declRefExpr().bind("expr"),
                                                  memberExpr().bind("expr"))))),
                  hasCastKind(CK_PointerToBoolean)))),
              unless(isInTemplateInstantiation()))
              .bind("if")),
      this);
}

static void checkImpl(const MatchFinder::MatchResult &Result, const Expr *Ref,
                      const IfStmt *If,
                      const ast_matchers::internal::Matcher<Expr> &RefMatcher,
                      CheckInterface &Check) {
  // Ignore macros.
  if (Ref->getBeginLoc().isMacroID())
    return;

  // Only allow variable accesses and member exprs for now, no function calls.
  // Check that we don't dereference the variable anywhere within the if. This
  // avoids false positives for checks of the pointer for nullptr before it is
  // dereferenced. If there is a dereferencing operator on this variable don't
  // emit a diagnostic. Also ignore array subscripts.
  auto foundDeref = match(findAll(unaryOperator(hasOperatorName("*"),
                                                hasUnaryOperand(RefMatcher))),
                          *If, *Result.Context)
                        .empty();
  auto foundArrDeref = match(findAll(arraySubscriptExpr(hasBase(RefMatcher))),
                             *If, *Result.Context)
                           .empty();

  auto foundCallArg =
      match(
          findAll(callExpr(hasAnyArgument(ignoringParenImpCasts(RefMatcher)))),
          *If, *Result.Context)
          .empty();

  auto foundDelExpr =
      match(
          findAll(cxxDeleteExpr(has(ignoringParenImpCasts(expr(RefMatcher))))),
          *If, *Result.Context)
          .empty();

  if (!foundDeref || !foundArrDeref || !foundCallArg || !foundDelExpr)
    return;

  auto &SM = Result.Context->getSourceManager();
  std::string msg = "Dubious check of `bool *" + Check.GetStrRepr(Ref, SM) +
                    "` against `nullptr`";
  Check.registerIssue(msg, Ref, SM);
}

void BoolPointerImplicitConversionCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *If = Result.Nodes.getNodeAs<IfStmt>("if");
  if (const auto *E = Result.Nodes.getNodeAs<Expr>("expr")) {
    const Decl *D = isa<DeclRefExpr>(E) ? cast<DeclRefExpr>(E)->getDecl()
                                        : cast<MemberExpr>(E)->getMemberDecl();
    const auto M =
        ignoringParenImpCasts(anyOf(declRefExpr(to(equalsNode(D))),
                                    memberExpr(hasDeclaration(equalsNode(D)))));
    checkImpl(Result, E, If, M, *this);
  }
}

} // namespace tidy
} // namespace clang
