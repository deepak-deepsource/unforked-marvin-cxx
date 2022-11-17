//===--- tools/extra/clang-tidy/ClangTidyDiagnosticConsumer.cpp ----------=== //
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
///  \file This file implements ClangTidyDiagnosticConsumer, Context
///  and ClangTidyError classes.
///
///  This tool uses the Clang Tooling infrastructure, see
///    http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html
///  for details on setting it up with LLVM source tree.
///
//===----------------------------------------------------------------------===//

#include <exception>
#include <fstream>
#include <iostream>

#include "engine/AnalyserContext.h"
#include "engine/CheckFactory.h"

#include "checks/AssignmentInIfConditionCheck.h"
#include "checks/BoolPointerImplicitConversionCheck.h"
#include "checks/CopyConstructorInitCheck.h"
#include "checks/DanglingHandleCheck.h"
#include "checks/FoldInitTypeCheck.h"
#include "checks/IntegerDivisionCheck.h"
#include "checks/InfiniteLoopCheck.h"
#include "checks/InaccurateEraseCheck.h"
#include "checks/ParentVirtualCallCheck.h"

using namespace clang;
using namespace tidy;
using namespace marvincxx;

// clang-format off
// turning the format off make it easier to visualise the listing of issues
/// register the generator for a new check
void AnalyserContext::registerChecks(void) {
  CheckFactory::RegisterCheck<AssignmentInIfConditionCheck>(
      "CXX-W2000", "assignment-in-if-condition");
  CheckFactory::RegisterCheck<IntegerDivisionCheck>(
      "CXX-W2001", "integer-division");
  CheckFactory::RegisterCheck<BoolPointerImplicitConversionCheck>(
      "CXX-W2002", "bool-pointer-implicit-conversion");
  CheckFactory::RegisterCheck<CopyConstructorInitCheck>(
      "CXX-W2003", "copy-constructor-init");
  CheckFactory::RegisterCheck<DanglingHandleCheck>(
      "CXX-W2004", "dangling-handle");
  CheckFactory::RegisterCheck<FoldInitTypeCheck>(
      "CXX-W2005", "fold-init-type");
  CheckFactory::RegisterCheck<InfiniteLoopCheck>(
      "CXX-W2006", "infinite-loop");
  CheckFactory::RegisterCheck<InaccurateEraseCheck>(
      "CXX-W2007", "inaccurate-erase");
  CheckFactory::RegisterCheck<ParentVirtualCallCheck>(
      "CXX-W2008", "parent-virtual-call");
}
// clang-format on

SmallVector<StringRef> AnalyserContext::getAllCheckNames(void) {
  return CheckFactory::getAllCheckNames();
}

std::string AnalyserContext::GetIssueCode(StringRef CheckName) {
  return CheckFactory::GetIssueCode(CheckName);
}
FilePoint AnalyserContext::getFilePoint(const SourceLocation Loc,
                                        const SourceManager &SM) {
  bool InValid;
  unsigned Line = SM.getPresumedLineNumber(Loc, &InValid);

  if (InValid)
    return FilePoint(0, 0);

  unsigned Column = SM.getPresumedColumnNumber(Loc, &InValid);
  return FilePoint(Line, Column);
}

FileRange AnalyserContext::getFileRange(const SourceLocation Begin,
                                        const SourceLocation End,
                                        const SourceManager &SM) {
  return FileRange(getFilePoint(Begin, SM), getFilePoint(End, SM));
}

StringRef AnalyserContext::getFilename(const SourceLocation SL,
                                       const SourceManager &SM) {
  return SM.getFilename(SL);
}

Location AnalyserContext::getLocation(const SourceLocation Begin,
                                      const SourceLocation End,
                                      const SourceManager &SM) {
  return Location(getFilename(Begin, SM).str(), getFileRange(Begin, End, SM));
}

Location AnalyserContext::getIssueLocation(const Stmt *S,
                                           const SourceManager &SM) {
  return getLocation(S->getBeginLoc(), S->getEndLoc(), SM);
}

Location AnalyserContext::getIssueLocation(const Decl *D,
                                           const SourceManager &SM) {
  return getLocation(D->getBeginLoc(), D->getEndLoc(), SM);
}

void AnalyserContext::registerIssue(std::string IssueCode,
                                    std::string IssueText,
                                    const Stmt *S,
                                    const SourceManager &SM) {
  auto Loc = getIssueLocation(S, SM);

  if (Loc.IsInvalid())
    std::cerr << "Skipping issue registration due to invalid location\n";
  else
    Issues.emplace_back(Issue(IssueCode, IssueText, Loc));
}

void AnalyserContext::registerIssue(std::string IssueCode,
                                    std::string IssueText, const Decl *D,
                                    const SourceManager &SM) {
  auto Loc = getIssueLocation(D, SM);
  if (!Loc.Path.empty())
    Issues.emplace_back(Issue(IssueCode, IssueText, Loc));
  else
    std::cerr << "Skipping issue registration due to missing filename"
              << std::endl;
}

void AnalyserContext::writeTo(std::ostream &out) {
  json JSonInstance = Issues;
  out << std::setw(2) << JSonInstance << std::endl;
}

void AnalyserContext::writeToJSON(StringRef OutputPath) {
  std::ofstream outfile{OutputPath.str()};
  writeTo(outfile);
}

void AnalyserContext::writeToStdout(void) { writeTo(std::cout); }

void AnalyserContext::setCheckNames(SmallVector<StringRef> CheckNames) {
  for (StringRef CheckName : CheckNames) {
    if (CheckFactory::GeneratorExist(CheckName) != true) {
      continue;
    }
    EnabledChecks.emplace_back(CheckName);
  }
}

void AnalyserContext::InitEnabledChecks(MatchFinder *F) {
  CheckFactory::InitEnabledChecks(this, F);
}

SmallVector<StringRef> AnalyserContext::getEnabledCheckNames(void) {
  return EnabledChecks;
}

AnalyserContext::AnalyserContext() {
  // Before the first translation unit we can get errors related to command-line
  // parsing, use empty string for the file name in this case.
  setCurrentFile("");
}

AnalyserContext::~AnalyserContext() = default;

void AnalyserContext::setCurrentFile(StringRef File) {
  CurrentFile = std::string(File);
}

void AnalyserContext::setASTContext(ASTContext *Context) {
  LangOpts = Context->getLangOpts();
}

bool AnalyserContext::isCheckEnabled(StringRef CheckName) const { return true; }
