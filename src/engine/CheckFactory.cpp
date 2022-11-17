#include <iostream>
#include <memory>

#include "CheckFactory.h"

std::vector<std::unique_ptr<CheckInterface>>
CheckFactory::InitChecks(std::vector<StringRef> enabledCheckNames,
                         AnalyserContext *Context, MatchFinder *F) {
  std::vector<std::unique_ptr<CheckInterface>> EnabledChecks;
  for (StringRef name : enabledCheckNames) {
    std::unique_ptr<CheckInterface> check =
        CheckName2Class[name](name, Context);
    check->registerMatchers(F);
    EnabledChecks.emplace_back(std::move(check));
  }
  return EnabledChecks;
}

std::vector<std::unique_ptr<CheckInterface>>
CheckFactory::InitEnabledChecks(AnalyserContext *Context, MatchFinder *F) {
  SmallVector<StringRef> EnabledChecks = Context->getEnabledCheckNames();
  std::vector<std::unique_ptr<CheckInterface>> CheckInstances;

  for (StringRef CheckName : EnabledChecks) {
    std::unique_ptr<CheckInterface> Check =
        CheckName2Class[CheckName](CheckName, Context);
    Check->registerMatchers(F);
    CheckInstances.emplace_back(std::move(Check));
  }

  return CheckInstances;
}

bool CheckFactory::GeneratorExist(StringRef CheckName) {
  return CheckName2Class.find(CheckName) != CheckName2Class.end();
}

SmallVector<StringRef> CheckFactory::getAllCheckNames(void) {
  SmallVector<StringRef> temp;

  for (auto Key : CheckName2Class.keys())
    temp.emplace_back(Key);

  return temp;
}

std::string CheckFactory::GetIssueCode(StringRef CheckName) {
  std::string IssueCode;
  if (CheckName2IssueCode.find(CheckName) == CheckName2IssueCode.end())
    std::cout << "Missing issue code for " + CheckName.str();
  else
    IssueCode = CheckName2IssueCode[CheckName];
  return IssueCode;
}
