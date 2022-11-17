//===--- InfiniteLoopCheck.h - clang-tidy -----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_INFINITELOOPCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_INFINITELOOPCHECK_H

#include "engine/CheckInterface.h"

namespace clang {
namespace tidy {

/// Finds obvious infinite loops (loops where the condition variable is
/// not changed at all).
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/bugprone/infinite-loop.html
class InfiniteLoopCheck : public marvincxx::CheckInterface {
public:
  InfiniteLoopCheck(StringRef Name, marvincxx::AnalyserContext *Context)
      : marvincxx::CheckInterface(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_INFINITELOOPCHECK_H
