#ifndef CITYFLOW_CONFIG_H
#define CITYFLOW_CONFIG_H

#include <deque>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <mutex>
#include <set>
#include <random>
#include <fstream>
#include <memory>
#include <list>
#include <map>
#include <queue>
#include <vector>
#include <cmath>
#include <typeinfo>
#include <stdexcept>
#include <utility>

#ifdef GTEST_PUBLIC
    #define private public
    #define protected public
#endif

namespace CityFlow {
    const int MAX_NUM_CARS_ON_SEGMENT = 10;
}

#endif //CITYFLOW_CONFIG_H
