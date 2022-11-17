//===--- UseNullptrCheck.h - clang-tidy--------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MODERNIZE_USE_NULLPTR_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MODERNIZE_USE_NULLPTR_H

#include "engine/CheckInterface.h"

namespace clang {
namespace tidy {

class UseNullptrCheck : public marvincxx::CheckInterface {
public:
  UseNullptrCheck(StringRef Name, marvincxx::AnalyserContext *EngineContext);
  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override {
    // FIXME this should be CPlusPlus11 but that causes test cases to
    // erroneously fail.
    return LangOpts.CPlusPlus;
  }
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  const StringRef NullMacrosStr;
  SmallVector<StringRef, 1> NullMacros;
};

} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MODERNIZE_USE_NULLPTR_H
