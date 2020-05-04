#ifndef CITYFLOW_CONFIG_H
#define CITYFLOW_CONFIG_H

#ifdef GTEST_PUBLIC
    #define private public
    #define protected public
#endif

namespace CityFlow {
    const int MAX_NUM_CARS_ON_SEGMENT = 10;
}

#endif //CITYFLOW_CONFIG_H
