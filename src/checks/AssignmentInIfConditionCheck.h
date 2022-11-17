//===--- AssignmentInIfConditionCheck.h - clang-tidy ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_ASSIGNMENTINIFCONDITIONCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_ASSIGNMENTINIFCONDITIONCHECK_H

#include "engine/AnalyserContext.h"
#include "engine/CheckInterface.h"

namespace clang {
namespace tidy {

/// Catches assignments within the condition clause of an if statement.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/bugprone-assignment-in-if-condition.html
class AssignmentInIfConditionCheck : public marvincxx::CheckInterface {
public:
  AssignmentInIfConditionCheck(StringRef Name,
                               marvincxx::AnalyserContext *Context)
      : CheckInterface(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  void report(const Expr *AssignmentExpr, SourceManager &SM);
};

} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_ASSIGNMENTINIFCONDITIONCHECK_H
