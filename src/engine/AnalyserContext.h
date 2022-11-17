//===--- ClangTidyDiagnosticConsumer.h - clang-tidy -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception //
//===----------------------------------------------------------------------===//

#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <vector>

#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/LangOptions.h"

#include "engine/Issue.hpp"
using namespace MCxxIssue;
using json = nlohmann::json;

namespace clang {
namespace marvincxx {

/// Every \c ClangTidyCheck reports errors through a \c DiagnosticsEngine
/// provided by this context.
///
/// A \c ClangTidyCheck always has access to the active context to report
/// warnings like:
/// \code
/// Context->Diag(Loc, "Single-argument constructors must be explicit")
///     << FixItHint::CreateInsertion(Loc, "explicit ");
/// \endcode
class AnalyserContext {
public:
  /// Initializes \c Context instance.
  AnalyserContext();

  ~AnalyserContext();

  /// Should be called when starting to process new translation unit.
  void setCurrentFile(StringRef File);

  /// Returns the main file name of the current translation unit.
  StringRef getCurrentFile() const { return CurrentFile; }

  /// Sets ASTContext for the current translation unit.
  void setASTContext(ASTContext *Context);

  /// Returns \c true if the check is enabled for the \c CurrentFile.
  ///
  /// The \c CurrentFile can be changed using \c setCurrentFile.
  bool isCheckEnabled(StringRef CheckName) const;

  /// Should be called when starting to process new translation unit.
  void setCurrentBuildDirectory(StringRef BuildDirectory) {
    CurrentBuildDirectory = BuildDirectory.str();
  }

  const LangOptions &getLangOpts() const { return LangOpts; }
  /// Returns build directory of the current translation unit.
  const std::string &getCurrentBuildDirectory() const {
    return CurrentBuildDirectory;
  }

  void registerIssue(std::string IssueCode, std::string IssueMessage,
                     const Stmt *Statement, const SourceManager &SM);

  void registerIssue(std::string IssueCode, std::string IssueText,
                     const Decl *D, const SourceManager &SM);

  void writeToJSON(StringRef OutputPath);

  void writeToStdout(void);

  void registerChecks(void);

  void setCheckNames(SmallVector<StringRef> CheckNames);

  SmallVector<StringRef> getAllCheckNames(void);

  void InitEnabledChecks(ast_matchers::MatchFinder *F);

  SmallVector<StringRef> getEnabledCheckNames();

  std::string GetIssueCode(StringRef CheckName);

private:
  FilePoint getFilePoint(const SourceLocation Loc, const SourceManager &SM);
  StringRef getFilename(const SourceLocation Loc, const SourceManager &SM);
  Location getLocation(const SourceLocation Begin, const SourceLocation End, const SourceManager &SM);
  FileRange getFileRange(const SourceLocation Begin, const SourceLocation End,
                         const SourceManager &SM);
  Location getIssueLocation(const Stmt *S, const SourceManager &SM);
  Location getIssueLocation(const Decl *D, const SourceManager &SM);

  void writeTo(std::ostream &out);

  std::string CurrentFile;
  LangOptions LangOpts;
  /// Returns the language options from the context.
  std::string CurrentBuildDirectory;
  std::vector<Issue> Issues;

  SmallVector<StringRef> EnabledChecks;
};
} // end namespace marvincxx
} // end namespace clang

#endif // CONTEXT_H
