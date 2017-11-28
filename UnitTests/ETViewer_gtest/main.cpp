#include "stdafx.h"
#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    int retval = RUN_ALL_TESTS();
    getchar(); // Wait for user input
    return retval;
}
