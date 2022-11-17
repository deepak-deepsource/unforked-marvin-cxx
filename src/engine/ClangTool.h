#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"

#include "engine/AnalyserContext.h"

namespace clang {

namespace marvincxx {

class MarvinCxxASTConsumer : public clang::ASTConsumer {
public:
  explicit MarvinCxxASTConsumer(AnalyserContext &Context)
      : EngineContext{Context} {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context);

private:
  AnalyserContext &EngineContext;
  ast_matchers::MatchFinder Finder;
};

class MarvinCxxFrontendAction : public ASTFrontendAction {
public:
  MarvinCxxFrontendAction(AnalyserContext &Context) : EngineContext{Context} {}

  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);

private:
  AnalyserContext &EngineContext;
};

class MarvinCxxActionFactory : public tooling::FrontendActionFactory {
public:
  MarvinCxxActionFactory(AnalyserContext &Context) : EngineContext{Context} {}

  std::unique_ptr<FrontendAction> create() override;

private:
  AnalyserContext &EngineContext;
};

} // namespace marvincxx
} // namespace clang
