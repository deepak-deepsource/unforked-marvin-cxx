#include "gtest/gtest.h"
#include "engine/AnalyserContext.h"

TEST(AnalyserContext, DefaultInit) {
  clang::marvincxx::AnalyserContext C;
  EXPECT_STREQ(C.getCurrentFile().str().c_str(), "");
}
