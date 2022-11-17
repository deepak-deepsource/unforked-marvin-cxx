#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/ADT/StringRef.h"

#include "engine/AnalyserContext.h"
#include "engine/CheckFactory.h"
#include "engine/ClangTool.h"

using namespace llvm;
using namespace clang::tooling;

namespace clang {
namespace marvincxx {
int Driver(CommonOptionsParser &OptionsParser, StringRef Checks,
           StringRef OutputPath);
}
} // namespace clang
