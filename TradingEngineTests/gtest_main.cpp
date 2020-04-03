/*
 * gtest_main.cpp
 *
 *  Created on: 24 Mar 2018
 *      Author: wesley
 */

// gtest_main.cpp
#include "gtest/gtest.h"
#include <stdio.h>

GTEST_API_ int main(int argc, char** argv) {
	printf("Running main() from gtest_main.cc\n");
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
