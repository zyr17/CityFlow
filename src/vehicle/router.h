#ifndef CITYFLOW_ROUTER
#define CITYFLOW_ROUTER

#include "engine/archive.h"

#include <vector>
#include <random>
#include <memory>

namespace CityFlow {
    class Road;
    class Drivable;
    class Route;
    class Lane;
    class LaneLink;
    class Vehicle;

    class Router {
    friend Archive;
    private:
        
        Vehicle* vehicle = nullptr;
        std::vector<Road *> route; // 经过路径。会使用updateShortestPath更新
        std::vector<Road *> anchorPoints; // 给定的边，必须依次经过
        std::vector<Road *>::const_iterator iCurRoad; //  所在道路的
        std::mt19937 *rnd = nullptr;

        mutable std::deque<Drivable *> planned; // TODO
        
        // 如果初始，随机选一个，否则选和当前车道序号最近的一个
        int selectLaneIndex(const Lane *curLane, const std::vector<Lane *> &lanes) const;

        // 用selectLaneIndex选LaneLink
        LaneLink *selectLaneLink(const Lane *curLane, const std::vector<LaneLink*> &laneLinks) const;

        // 刚出发时用selectLaneIndex选择一个Lane
        Lane *selectLane(const Lane *curLane, const std::vector<Lane *> &lanes) const;

        enum class RouterType{
            LENGTH,
            DURATION,
            DYNAMIC // TODO: dynamic routing
        };

        RouterType type = RouterType::DURATION;

    public:

        Router(const Router &other);

        // 给出车辆，规定的Route以及随机数
        Router(Vehicle *vehicle, std::shared_ptr<const Route> route, std::mt19937 *rnd);

        Road *getFirstRoad() {
            return anchorPoints[0];
        }

        Drivable *getFirstDrivable() const;

        // 从最近已行驶的路径planned中对应id找下一个
        Drivable *getNextDrivable(size_t i = 0) const;

        // 给定一个Drivable找到路径规划中下一个Drivable在哪里
        Drivable *getNextDrivable(const Drivable *curDrivable) const;

        // 更新iCurRoad，清理已经经过的planned
        // BUG: 当车辆处于LaneLink的时候不会更新iCurRoad。这会导致getValidLane出错
        void update();

        bool isLastRoad(const Drivable *drivable) const;

        bool onLastRoad() const;

        // 是否在合法车道上。仅会被Vehicle::getNextSpeed当laneChange时调用。
        bool onValidLane()  const{
            return !(getNextDrivable() == nullptr && !onLastRoad());
        }

        // 假设curLane在iCurRoad上，在当前车道寻找能到下一条路的，离当前车道最近的车道，包括自己
        Lane * getValidLane(const Lane *curLane) const;

        void setVehicle(Vehicle *vehicle) {
            this->vehicle = vehicle;
        }

        // 给定起点终点，找到路径并把路径写到buffer中。成功找到返回true否则false
        bool dijkstra(Road *start, Road *end, std::vector<Road *> &buffer);

        // 更新自己的route
        bool updateShortestPath();
    };
}

#endif