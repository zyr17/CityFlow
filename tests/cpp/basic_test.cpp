#include <string>
#include <cstdlib>
#include <gtest/gtest.h>

#include "engine/engine.h"

using namespace CityFlow;

size_t threads = std::min(std::thread::hardware_concurrency(), 4u);
std::string exampleConfig = "../json/example/config.json";
std::string allpassConfig = "../json/allpass/config.json";
std::string directionchangeConfig = "../json/directionchange/config.json";
std::string linkallConfig = "../json/linkall/config.json";
std::string oneroadConfig = "../json/oneroad/config.json";
std::string configFile = exampleConfig;

extern GTestInjectSwitchClass GTestInjectSwitch;

TEST(Basic, Basic) {
    size_t totalStep = 2000;

    Engine engine(configFile, threads);
    for (size_t i = 0; i < totalStep; i++) {
        engine.nextStep();
    }
    SUCCEED();
}

TEST(Basic, Macro) {
#ifdef GTEST_PUBLIC
    SUCCEED();
#else
    FAIL();
#endif
}

TEST(Basic, Private){
    Engine engine(configFile, threads);
    ASSERT_EQ(engine.threadNum, threads);
    engine.nextStep();
    SUCCEED();
}

TEST(Basic, API) {
    size_t totalStep = 200;

    Engine engine(configFile, threads);
    for (size_t i = 0; i < totalStep; i++) {
        engine.nextStep();
        engine.getVehicleSpeed();
        engine.getLaneVehicles();
        engine.getLaneWaitingVehicleCount();
        engine.getVehicleDistance();
        engine.getCurrentTime();
        engine.getVehicleCount();
    }
    SUCCEED();
}

TEST(Basic, reset) {
    size_t totalStep = 200;

    Engine engine(configFile, threads);
    for (size_t i = 0; i < totalStep; i++) {
        engine.nextStep();
    }
    double curTime = engine.getCurrentTime();
    size_t vehCnt = engine.getVehicleCount();
    engine.reset(true);
    for (size_t i = 0; i < totalStep; i++) {
        engine.nextStep();
    }
    EXPECT_EQ(engine.getCurrentTime(), curTime);
    EXPECT_EQ(engine.getVehicles().size(), vehCnt);
    SUCCEED();
}

namespace InvalidLaneLaneChangeTest {

    int InvalidLaneLaneChangeStep(Engine& engine) {
        int assertTimes = 0;
        engine.nextStep();
        for (auto& a : engine.threadDrivablePool)
            for (auto b : a) {
                Vehicle* prev = nullptr;
                auto l = b->getVehicles();
                for (auto v : l) {
                    if (prev) {
                        assert(prev->getDistance() > v->getDistance() || prev->getDistance() == v->getDistance() && prev->getPriority() > v->getPriority());
                        assert(v->getLeader() == prev);
                        assert(prev->getFollower() == v);
                        assertTimes += 3;
                    }
                    prev = v;
                }
                assert(!b->getLastVehicle() || !b->getLastVehicle()->getFollower());
                assertTimes++;
                if (b->isLane()) {
                    Lane* ll = static_cast<Lane*>(b);
                    for (auto s : ll->getSegments()) {
                        prev = nullptr;
                        for (auto v : s.getVehicles()) {
                            if (prev) {
                                double dis = (*v)->getDistance();
                                assert(prev->getDistance() > dis || prev->getDistance() == dis && prev->getPriority() > (*v)->getPriority());
                                assert(dis >= s.getStartPos() && dis <= s.getEndPos());
                                assertTimes += 2;
                            }
                            prev = *v;
                        }
                    }
                }
            }
        return assertTimes;
    }

    void pushVehicle(Engine& engine, std::vector<std::string>& roads, double maxspeed = 16.67, double posAcc = 4.5, double negAcc = 2.0, double headwayTime = 1.5) {
        std::map<std::string, double> vt;
        vt["length"] = 5.0;
        vt["width"] = 2.0;
        vt["maxPosAcc"] = posAcc;
        vt["maxNegAcc"] = negAcc;
        vt["usualPosAcc"] = posAcc;
        vt["usualNegAcc"] = negAcc;
        vt["minGap"] = 2.5;
        vt["maxSpeed"] = maxspeed;
        vt["headwayTime"] = headwayTime;
        engine.pushVehicle(vt, roads);
    }

    void neverStopRandomSpeed(Engine& engine, std::vector<std::string>& roads, double minSpeed = 1, double maxSpeed = 30) {
        double speed = minSpeed + rand() * 1.0 / RAND_MAX * (maxSpeed - minSpeed);
        pushVehicle(engine, roads, speed, 999999999, 0, 0);
    }

    TEST(InvalidLaneLaneChange, Basic) {
        size_t totalStep = 1000;
        Engine engine(exampleConfig, threads);
        int assertTimes = 0;
        for (size_t i = 0; i < totalStep; i++) {
            assertTimes += InvalidLaneLaneChangeStep(engine);
        }
        std::cout << "assert times: " << assertTimes << std::endl;
        SUCCEED();
    }

    TEST(InvalidLaneLaneChange, FixedSpeed) {
        size_t totalStep = 10000;
        Engine engine(oneroadConfig, threads);
        int assertTimes = 0;
        std::vector<std::string> vec = { "road_0", "road_1" };
        for (size_t i = 0; i < totalStep; i++) {
            neverStopRandomSpeed(engine, vec);
            assertTimes += InvalidLaneLaneChangeStep(engine);
        }
        std::cout << "assert times: " << assertTimes << std::endl;
        SUCCEED();
    }
    
    TEST(InvalidLaneLaneChange, RandomStart) {
        GTestInjectSwitch.Router_getFirstDrivable_random = true;
        size_t totalStep = 1000;
        Engine engine(exampleConfig, threads);
        int assertTimes = 0;
        for (size_t i = 0; i < totalStep; i++) {
            assertTimes += InvalidLaneLaneChangeStep(engine);
        }
        std::cout << "assert times: " << assertTimes << std::endl;
        GTestInjectSwitch.clear();
        SUCCEED();
    }

}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}