#include "Driver.h"

///
/// Allocate the check name to generator map
///
llvm::StringMap<CheckFactory::CheckGenerator> CheckFactory::CheckName2Class =
    [] { return llvm::StringMap<CheckFactory::CheckGenerator>(); }();

llvm::StringMap<std::string> CheckFactory::CheckName2IssueCode = {};

SmallVector<StringRef> splitArgChecks(StringRef Checks) {
  SmallVector<StringRef> CheckNames;
  char Seperator = ';';
  StringRef(Checks).split(CheckNames, Seperator, -1, false);
  return CheckNames;
}

int clang::marvincxx::Driver(CommonOptionsParser &OptionsParser,
                             StringRef Checks, StringRef OutputPath) {
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  AnalyserContext EngineContext;
  EngineContext.registerChecks();

  SmallVector<StringRef> CheckNames;
  if (Checks.equals("all"))
    CheckNames = EngineContext.getAllCheckNames();
  else
    CheckNames = splitArgChecks(Checks);

  EngineContext.setCheckNames(CheckNames);

  MarvinCxxActionFactory Factory(EngineContext);
  auto Result = Tool.run(&Factory);

  if (OutputPath.equals(""))
    EngineContext.writeToStdout();
  else
    EngineContext.writeToJSON(OutputPath);

  return Result;
}
