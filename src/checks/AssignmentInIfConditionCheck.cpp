//===--- AssignmentInIfConditionCheck.cpp - clang-tidy --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AssignmentInIfConditionCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {

void AssignmentInIfConditionCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(translationUnitDecl(), this);
}

void AssignmentInIfConditionCheck::check(
    const ast_matchers::MatchFinder::MatchResult &Result) {
  class Visitor : public RecursiveASTVisitor<Visitor> {
    AssignmentInIfConditionCheck &Check;
    SourceManager &SM;

  public:
    explicit Visitor(AssignmentInIfConditionCheck &Check, SourceManager &SM)
        : Check(Check), SM(SM) {}
    bool VisitIfStmt(IfStmt *If) {
      class ConditionVisitor : public RecursiveASTVisitor<ConditionVisitor> {
        AssignmentInIfConditionCheck &Check;
        SourceManager &SM;

      public:
        explicit ConditionVisitor(AssignmentInIfConditionCheck &Check,
                                  SourceManager &SM)
            : Check(Check), SM(SM) {}

        // Dont traverse into any lambda expressions.
        bool TraverseLambdaExpr(LambdaExpr *, DataRecursionQueue * = nullptr) {
          return true;
        }

        bool VisitBinaryOperator(BinaryOperator *BO) {
          if (BO->isAssignmentOp())
            Check.report(BO, SM);
          return true;
        }

        bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr *OCE) {
          if (OCE->isAssignmentOp())
            Check.report(OCE, SM);
          return true;
        }
      };

      ConditionVisitor(Check, SM).TraverseStmt(If->getCond());
      return true;
    }
  };
  Visitor(*this, Result.Context->getSourceManager())
      .TraverseAST(*Result.Context);
}

void AssignmentInIfConditionCheck::report(const Expr *AssignmentExpr,
                                          SourceManager &SM) {
  std::string msg = "Found assignment `" + GetStrRepr(AssignmentExpr, SM) +
                    "` in `if` condition";
  registerIssue(msg, AssignmentExpr, SM);
}

} // namespace tidy
} // namespace clang
