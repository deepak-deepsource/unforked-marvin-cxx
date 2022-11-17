#ifndef CHECK_INTERFACE
#define CHECK_INTERFACE

#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/Optional.h"

#include "engine/AnalyserContext.h"

namespace clang {
class SourceManager;

namespace marvincxx {

class CheckInterface : public ast_matchers::MatchFinder::MatchCallback {
public:
  CheckInterface(StringRef CheckName, AnalyserContext *EngineContext);

  virtual bool isLanguageVersionSupported(const LangOptions &LangOpts) const {
    return true;
  }

  virtual void registerPPCallbacks(const SourceManager &SM, Preprocessor *PP,
                                   Preprocessor *ModuleExpanderPP) {}

  virtual void registerMatchers(ast_matchers::MatchFinder *Finder) {}

  virtual void check(const ast_matchers::MatchFinder::MatchResult &Result) {}

  void registerIssue(std::string IssueText, const Stmt *S,
                     const SourceManager &SM);

  // This is an overloaded function
  void registerIssue(std::string IssueText, const Decl *D,
                     const SourceManager &SM);

  std::string GetStrRepr(const Stmt *S, const SourceManager &SM);
  std::string GetStrRepr(const Decl *D, const SourceManager &SM);

private:
  PrintingPolicy getPrintPolicy(void);
  void run(const ast_matchers::MatchFinder::MatchResult &Result) override;
  StringRef getID() const override { return CheckName; }
  StringRef CheckName;
  AnalyserContext *EngineContext;

protected:
  StringRef getCurrentMainFile() const {
    assert(EngineContext != nullptr);
    return EngineContext->getCurrentFile();
  }
};

} // namespace marvincxx
} // namespace clang

#endif // CHECK_INTERFACE
