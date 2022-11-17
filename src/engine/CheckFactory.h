#ifndef MCXX_CHECK_FACTORY_H
#define MCXX_CHECK_FACTORY_H
#include "engine/CheckInterface.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

// TODO: Declare the namespace marvincxx here
using namespace clang;
using namespace marvincxx;
using namespace ast_matchers; // for MatchFinder

class CheckFactory {
  using CheckGenerator = std::function<std::unique_ptr<CheckInterface>(
      StringRef, AnalyserContext *)>;

public:
  template <typename CheckClass>
  static void RegisterCheck(std::string IssueCode, llvm::StringRef checkName) {
    CheckGenerator generator = [](StringRef checkName,
                                  AnalyserContext *Context) {
      return std::make_unique<CheckClass>(checkName, Context);
    };
    CheckName2Class[checkName] = generator;
    CheckName2IssueCode[checkName] = IssueCode;
  }

  static std::vector<std::unique_ptr<CheckInterface>>
  InitChecks(std::vector<StringRef>, AnalyserContext *, MatchFinder *);

  static void RegisterChecks(void);

  static std::vector<std::unique_ptr<CheckInterface>>
  InitEnabledChecks(AnalyserContext *Context, MatchFinder *F);

  static bool GeneratorExist(StringRef CheckName);

  static SmallVector<StringRef> getAllCheckNames(void);

  static std::string GetIssueCode(StringRef CheckName);

private:
  static llvm::StringMap<CheckGenerator> CheckName2Class;
  static llvm::StringMap<std::string> CheckName2IssueCode;
};
#endif
