#include "engine/engine.h"
#include "utility/optionparser.h"

#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace CityFlow;

int main(int argc, char const *argv[]) {
    optionparser::OptionParser parser;

    parser.add_option("--configFile", "-c")
            .help("config file")
            .mode(optionparser::StorageMode::STORE_VALUE)
            .required(true);

    parser.add_option("--totalStep", "-s")
            .help("simulation steps")
            .default_value(1000)
            .mode(optionparser::StorageMode::STORE_VALUE);

    parser.add_option("--threadNum", "-t")
            .help("number of threads")
            .default_value(1)
            .mode(optionparser::StorageMode::STORE_VALUE);

    parser.add_option("--dataDir", "-d")
            .help("data directory")
            .default_value("./")
            .mode(optionparser::StorageMode::STORE_VALUE);

    parser.add_option("--verbose", "-v")
            .help("be verbose")
            .mode(optionparser::StorageMode::STORE_TRUE);

    parser.eat_arguments(argc, argv);
    std::string configFile = parser.get_value<std::string>("configFile");
    bool verbose = parser.get_value<bool>("verbose");
    size_t totalStep = parser.get_value<int>("totalStep");
    size_t threadNum = parser.get_value<int>("threadNum");
    std::string dataDir = parser.get_value<std::string>("dataDir");

    srand(unsigned(time(NULL)));
    Engine engine(dataDir + configFile, (size_t) threadNum);
    time_t startTime, endTime;
    double dummy = 0;
    time(&startTime);
    std::vector<std::string> vec = { "road_0", "road_1" };
    for (auto& i : engine.getDirectionChangeLanes()) std::cout << i.first << '\n';
    engine.setLaneDirection("road_1_2_3_1", "turn_left");
    for (int i = 0; i < totalStep; i++) {
        if (verbose) {
            std::cout << i << " " << engine.getVehicleCount() << std::endl;
        }
        if (i % 1000 == 0) {
            std::string d = i % 2000 ? "go_straight" : "turn_left";
            engine.setLaneDirection("road_0_1_0_1", d);
            engine.setLaneDirection("road_1_2_3_0", d);
        }
        //neverStopRandomSpeed(engine, vec);
        engine.nextStep();
        for (auto& i : engine.getVehicleSpeed())
            dummy += i.second;
        for (auto& i : engine.getLaneVehicles())
            dummy += i.second.size();
        for (auto& i : engine.getLaneWaitingVehicleCount())
            dummy += i.second;
        for (auto& i : engine.getVehicleDistance())
            dummy += i.second;
        dummy += engine.getCurrentTime();
    }
    time(&endTime);
    std::cout << "Total Step: " << totalStep << std::endl;
    std::cout << "Total Time: " << (endTime - startTime) << "s" << std::endl;
    std::cout << "Dummy: " << dummy << std::endl;
    return 0;
}