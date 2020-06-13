#include <string>
#include <cstdlib>
#include <gtest/gtest.h>

#include "engine/engine.h"

using namespace CityFlow;

size_t threads = std::min(std::thread::hardware_concurrency(), 16u);

std::vector<std::string> exampleFolders = {
    "example",
    "allpass",
    "directionchange",
    "linkall",
    "oneroad",
    "floatendflow",
    "dense",
};

std::string configFilePos(std::string folder) {
    return "../json/" + folder + "/config.json";
}

std::string configFile = configFilePos("example");

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

TEST(Basic, allExamples) {
    size_t totalStep = 1000;

    for (auto folder : exampleFolders) {
        printf("----- Evaluating Example %s -----\n", folder.c_str());
        Engine engine(configFilePos(folder), threads);
        for (size_t i = 0; i < totalStep; i++)
            engine.nextStep();
        printf("Finished vehicles: %d, Running vehicles: %d\n", engine.finishedVehicleCnt, engine.getVehicleCount());
    }
    SUCCEED();
}

TEST(Basic, threads) {
    size_t totalStep = 3000;
    int step = threads / 4;
    step = step ? step : 1;
    
    for (int thread = 1; thread <= threads; thread += step) {
        auto startTime = clock();
        Engine engine(configFilePos("dense"), thread);
        engine.setSaveReplay(false);
        for (size_t i = 0; i < totalStep; i++)
            engine.nextStep();
        printf("Thred Number: %d, Time: %.3f sec\n", thread, (clock() - startTime) * 1.0 / CLOCKS_PER_SEC);
    }
    SUCCEED();
}

std::string randString(int length = 32) {
    std::string basic = "0123456789qazwsxedcrfvtgbyhnujmikolp";
    std::string res;
    for (; length; length--)
        res += basic[rand() % basic.size()];
    return res;
}

TEST(Basic, changeLogFile) {
    size_t totalStep = 200;
    std::string roadFile = randString(), logFile = randString();
    std::string logFile2 = randString();

    {
        Engine engine(configFile, threads);

        engine.setLogFile(roadFile, logFile);

        for (size_t i = 0; i < totalStep; i++) {
            engine.nextStep();
        }
    }

    FILE *roadf = fopen(roadFile.c_str(), "r"), *logf = fopen(logFile.c_str(), "r");
    EXPECT_TRUE(roadf);
    EXPECT_TRUE(logf);
    fclose(roadf);
    fclose(logf);

    std::string e2dir;

    {
        Engine engine2(configFile, threads);
        e2dir = engine2.dir;

        engine2.setReplayLogFile(logFile2);

        for (size_t i = 0; i < totalStep; i++) {
            engine2.nextStep();
        }
    }

    FILE *logf2 = fopen(logFile.c_str(), "r");
    EXPECT_TRUE(logf);
    fclose(logf2);

    std::string windows = "powershell.exe -Command \"rm -force %s\"";
    std::string linux = "bash -c \"rm -f %s\"";

    std::string cmd;
#ifdef WIN32
    cmd = windows;
#else
    cmd = linux;
#endif

    char buffer[150];

    sprintf(buffer, cmd.c_str(), roadFile.c_str());
    system(buffer);
    sprintf(buffer, cmd.c_str(), logFile.c_str());
    system(buffer);
    sprintf(buffer, cmd.c_str(), (e2dir + logFile2).c_str());
    system(buffer);

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
        Engine engine(configFilePos("example"), threads);
        int assertTimes = 0;
        for (size_t i = 0; i < totalStep; i++) {
            assertTimes += InvalidLaneLaneChangeStep(engine);
        }
        std::cout << "assert times: " << assertTimes << std::endl;
        SUCCEED();
    }

    TEST(InvalidLaneLaneChange, FixedSpeed) {
        size_t totalStep = 10000;
        Engine engine(configFilePos("oneroad"), threads);
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
        Engine engine(configFilePos("example"), threads);
        int assertTimes = 0;
        for (size_t i = 0; i < totalStep; i++) {
            assertTimes += InvalidLaneLaneChangeStep(engine);
        }
        std::cout << "assert times: " << assertTimes << std::endl;
        GTestInjectSwitch.clear();
        SUCCEED();
    }

}

namespace DirectionChangeLanesTest {
    TEST(DirectionChangeLanesTest, Basic) {
        size_t totalStep = 10000;
        int assertTimes = 0;
        Engine engine(configFilePos("directionchange"), threads);
        for (size_t i = 0; i < totalStep; i++) {
            if (i % 1000 == 0)
                engine.setLaneDirection("road_0_1_0_2", i % 2000 ? "go_straight" : "turn_left");
            engine.nextStep();
            for (auto lane : engine.roadnet.getLanes())
                if (lane->isDirectionChangeLane()) {
                    auto nowDirection = lane->getActivatedDirection();
                    for (auto lanelink : lane->getLaneLinks()) {
                        assert(lanelink->isActivated() == (nowDirection == lanelink->getRoadLinkType()));
                        assertTimes++;
                    }
                }
                else {
                    for (auto lanelink : lane->getLaneLinks()) {
                        assert(lanelink->isActivated());
                        assertTimes++;
                    }
                }
        }
        std::cout << "assert times: " << assertTimes << std::endl;
        SUCCEED();
    }
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}