//===--- BoolPointerImplicitConversionCheck.h - clang-tidy ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_BOOLPOINTERIMPLICITCONVERSIONCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_BOOLPOINTERIMPLICITCONVERSIONCHECK_H

#include "engine/AnalyserContext.h"
#include "engine/CheckInterface.h"

namespace clang {
namespace tidy {

/// Checks for conditions based on implicit conversion from a bool pointer to
/// bool.
///
/// Example:
///
/// \code
///   bool *p;
///   if (p) {
///     // Never used in a pointer-specific way.
///   }
/// \endcode
class BoolPointerImplicitConversionCheck : public marvincxx::CheckInterface {
public:
  BoolPointerImplicitConversionCheck(StringRef Name,
                                     marvincxx::AnalyserContext *Context)
      : CheckInterface(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_BOOLPOINTERIMPLICITCONVERSIONCHECK_H
