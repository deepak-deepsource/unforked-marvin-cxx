//===--- IntegerDivisionCheck.h - clang-tidy---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_INTEGER_DIVISION_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_INTEGER_DIVISION_H

#include "engine/CheckInterface.h"

namespace clang {
namespace tidy {

/// Finds cases where integer division in a floating point context is likely to
/// cause unintended loss of precision.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/bugprone/integer-division.html
class IntegerDivisionCheck : public marvincxx::CheckInterface {
public:
  IntegerDivisionCheck(StringRef Name, marvincxx::AnalyserContext *Context)
      : CheckInterface(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_INTEGER_DIVISION_H
