#include "gtest/gtest.h"
#include "engine/Issue.hpp"

using namespace MCxxIssue;

TEST(FilePoint, OnlyCtorInit) {
  const FilePoint P{1, 2};
  EXPECT_EQ(P.Line, 1);
  EXPECT_EQ(P.Column, 2);
}
