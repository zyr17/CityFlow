#ifndef CITYFLOW_TRAFFICLIGHT_H
#define CITYFLOW_TRAFFICLIGHT_H

#include <vector>

namespace CityFlow {
    class Intersection;

    class RoadLink;

    class RoadNet;

    class TrafficLight;

    class LightPhase {
        friend class RoadNet;
        friend class RoadLink;
        friend class TrafficLight;
    private:
        unsigned int phase = 0; // 当前阶段ID
        double time = 0.0; // 持续时间
        std::vector<bool> roadLinkAvailable;  // 该phase启用时每个roadLink的可用性
    };

    class TrafficLight {
        friend class RoadNet;
        friend class Archive;
    private:
        Intersection *intersection = nullptr; // 信号灯所在路口
        std::vector<LightPhase> phases; // 依次控制phase
        std::vector<int> roadLinkIndices; // not used
        double remainDuration = 0.0; // 剩余时间
        int curPhaseIndex = 0; // 当前阶段index
    public:
        void init(int initPhaseIndex); // 初始化到第i个阶段

        int getCurrentPhaseIndex();

        LightPhase &getCurrentPhase();

        Intersection &getIntersection();

        std::vector<LightPhase> &getPhases();

        void passTime(double seconds); // 经过若干秒。如果时间到就换信号灯。如果phase时间和tick不对齐会丢失

        void setPhase(int phaseIndex);

        void reset(); // 初始化到第0个阶段
    };
}

#endif //CITYFLOW_TRAFFICLIGHT_H