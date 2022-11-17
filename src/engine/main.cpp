#include "engine/Driver.h"
#include "clang/Tooling/CommonOptionsParser.h"

using namespace clang::tooling;
using namespace clang::marvincxx;

static llvm::cl::OptionCategory CxxAnalyserEngine("marvincxx options");
static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static llvm::cl::opt<std::string> Checks(
    "checks",
    llvm::cl::desc(
       R"(name of checks to enable; skip this option to enable all checks)"),
    llvm::cl::init("all"), llvm::cl::cat(CxxAnalyserEngine));
static llvm::cl::opt<std::string> OutputPath(
    "output",
    llvm::cl::desc(R"(path to write the found issues in JSON format)"),
    llvm::cl::init(""), llvm::cl::cat(CxxAnalyserEngine));

int main(int argc, const char **argv) {
  auto ExpectedParser =
      CommonOptionsParser::create(argc, argv, CxxAnalyserEngine);

  if (!ExpectedParser) {
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionsParser = ExpectedParser.get();

  return Driver(OptionsParser, Checks, OutputPath);
}
