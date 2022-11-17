#include <memory>

#include "clang/AST/ASTContext.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/ADT/StringMap.h"

#include "engine/CheckFactory.h"
#include "engine/ClangTool.h"

using namespace clang;

void MarvinCxxASTConsumer::HandleTranslationUnit(ASTContext &Context) {
  /// Register all checks -- this creates a mapping from check name
  /// to a check instance generator

  auto EnabledChecks = CheckFactory::InitEnabledChecks(&EngineContext, &Finder);
  // Run the checks
  Finder.matchAST(Context);
}

std::unique_ptr<clang::ASTConsumer>
MarvinCxxFrontendAction::CreateASTConsumer(clang::CompilerInstance &Compiler,
                                           llvm::StringRef InFile) {
  EngineContext.setASTContext(&Compiler.getASTContext());
  return std::make_unique<MarvinCxxASTConsumer>(EngineContext);
}

std::unique_ptr<FrontendAction> MarvinCxxActionFactory::create() {
  return std::make_unique<MarvinCxxFrontendAction>(EngineContext);
}
