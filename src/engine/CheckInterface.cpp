#include "engine/CheckInterface.h"
#include "clang/AST/PrettyPrinter.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"

namespace clang {
namespace marvincxx {

CheckInterface::CheckInterface(StringRef CheckName,
                               AnalyserContext *EngineContext)
    : CheckName(CheckName), EngineContext(EngineContext) {
  assert(EngineContext != nullptr);
  assert(!CheckName.empty());
}

void CheckInterface::run(const ast_matchers::MatchFinder::MatchResult &Result) {
  // For historical reasons, checks don't implement the MatchFinder run()
  // callback directly. We keep the run()/check() distinction to avoid interface
  // churn, and to allow us to add cross-cutting logic in the future.
  check(Result);
}

PrintingPolicy CheckInterface::getPrintPolicy(void) {
  PrintingPolicy PP{EngineContext->getLangOpts()};
  PP.FullyQualifiedName = 1;
  PP.SuppressScope = 0;
  PP.PrintCanonicalTypes = 1;
  PP.SuppressImplicitBase = 1;
  PP.TerseOutput = 1;
  PP.IncludeNewlines = 0;

  return PP;
}

std::string CheckInterface::GetStrRepr(const Stmt *S, const SourceManager &SM) {
  std::string Text;
  llvm::raw_string_ostream stream(Text);
  S->printPretty(stream, nullptr, getPrintPolicy());
  stream.flush();

  return Text;
}

std::string CheckInterface::GetStrRepr(const Decl *D, const SourceManager &SM) {
  std::string Text;

  llvm::raw_string_ostream stream(Text);
  D->print(stream, getPrintPolicy());
  stream.flush();

  return Text;
}

void CheckInterface::registerIssue(std::string IssueMessage, const Stmt *S,
                                   const SourceManager &SM) {
  std::string IssueCode = EngineContext->GetIssueCode(CheckName);
  if (IssueCode.empty())
    std::cerr << "Missing IssueCode! Not adding issue\n";
  else
    EngineContext->registerIssue(IssueCode, IssueMessage, S, SM);
}

void CheckInterface::registerIssue(std::string IssueMessage, const Decl *D,
                                   const SourceManager &SM) {
  std::string IssueCode = EngineContext->GetIssueCode(CheckName);
  if (IssueCode.empty())
    std::cerr << "Missing IssueCode! Not adding issue\n";
  else
    EngineContext->registerIssue(IssueCode, IssueMessage, D, SM);
}

} // namespace marvincxx
} // namespace clang
