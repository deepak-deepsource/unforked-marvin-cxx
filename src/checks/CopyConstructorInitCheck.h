//===--- CopyConstructorInitCheck.h - clang-tidy--------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_COPY_CONSTRUCTOR_INIT_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_COPY_CONSTRUCTOR_INIT_H

#include "engine/CheckInterface.h"

using namespace clang::marvincxx;

namespace clang {
namespace tidy {

/// Finds copy constructors where the ctor don't call the copy constructor of
/// the base class.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/misc/copy-constructor-init.html
class CopyConstructorInitCheck : public CheckInterface {
public:
  CopyConstructorInitCheck(StringRef Name, AnalyserContext *Context)
      : CheckInterface(Name, Context) {}
  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override {
    return LangOpts.CPlusPlus;
  }
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_COPY_CONSTRUCTOR_INIT_H
