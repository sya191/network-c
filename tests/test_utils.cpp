#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <math.h>
extern "C" {
#include "utils.h"
}
TEST(testUtils, getTimeStamp) {
    double time = 0.1;
    double previous = get_timestamp();
    std::this_thread::sleep_for(std::chrono::duration<double>(time));
    double diff = get_timestamp() - previous;
    double precision = time - diff;
    ASSERT_LT(std::fabs(precision), 0.01);
}