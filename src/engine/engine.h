#ifndef CITYFLOW_ENGINE_H
#define CITYFLOW_ENGINE_H

#include "flow/flow.h"
#include "roadnet/roadnet.h"
#include "engine/archive.h"
#include "utility/barrier.h"

#include <mutex>
#include <thread>
#include <set>
#include <random>
#include <fstream>


namespace CityFlow {

    class Engine {
        friend class Archive;
    private:

        static bool vehicleCmp(const std::pair<Vehicle *, double> &a, const std::pair<Vehicle *, double> &b) {
            return a.second > b.second;
        }

        // vehicle池 int为在第几线程处理
        std::map<int, std::pair<Vehicle *, int>> vehiclePool;
        std::map<std::string, Vehicle *> vehicleMap; // ID -> vehicle
        std::vector<std::set<Vehicle *>> threadVehiclePool;
        std::vector<std::vector<Road *>> threadRoadPool;
        std::vector<std::vector<Intersection *>> threadIntersectionPool;
        std::vector<std::vector<Drivable *>> threadDrivablePool;
        std::vector<Flow> flows; // 周期产生新车辆的Flow
        RoadNet roadnet;
        int threadNum;
        double interval; // 模拟周期，秒为单位
        bool saveReplay;
        bool saveReplayInConfig; // saveReplay option in config json
        bool warnings; // 是否对参数进行范围检查，提示不合理的范围。代码中永远为false

        /*多线程计算车辆情况后收集结果*/
        std::vector<std::pair<Vehicle *, double>> pushBuffer; // 多线程更新改变了Drivable了的车辆。
        std::vector<Vehicle *> laneChangeNotifyBuffer; // 准备变道的车辆
        std::set<Vehicle *> vehicleRemoveBuffer; // 到终点需要移除的车辆
        /////////

        rapidjson::Document jsonRoot;
        std::string stepLog; // empty

        size_t step = 0;
        size_t activeVehicleCount = 0;
        int seed;
        std::mutex lock;
        Barrier startBarrier, endBarrier;
        std::vector<std::thread> threadPool;
        bool finished = false;
        std::string dir;
        std::ofstream logOut;

        bool rlTrafficLight;
        bool laneChange;
        int manuallyPushCnt = 0;

        int finishedVehicleCnt = 0;
        double cumulativeTravelTime = 0;

    private:
        void vehicleControl(Vehicle &vehicle, 
            std::vector<std::pair<Vehicle *, double>> &buffer); // 车辆控制。车辆的移动、变道、消失等计算

        void planRoute(); // 检查要重新规划路线车辆的规划路线。规划出错移除

        void getAction(); // sync

        void updateAction(); // sync 清空vehicleRemoveBuffer

        void updateLocation(); // sync 添加到了新的Drivable的车辆。

        void updateLeaderAndGap(); // sync

        void planLaneChange(); // sync 调用scheduleLaneChange

        void handleWaiting(); // 检查是否能够将等待入列的车辆加入车道

        void updateLog();

        bool checkWarning(); // 对一些参数范围进行检查

        bool loadRoadNet(const std::string &jsonFile);

        bool loadFlow(const std::string &jsonFilename);

        std::vector<const Vehicle *> getRunningVehicles(bool includeWaiting=false) const;

        void scheduleLaneChange(); // 从laneChangeNotifyBuffer中对准备变道的车辆生成影子

        void insertShadow(Vehicle *vehicle); // 添加影子车辆

        /* 多线程对应函数 */
        // 多线程逻辑：使用两个Barrier，共计threadNum+1个线程同步。
        // 子线程由threadController作为入口，并在收到终止信号前永远循环。
        // 子线程内部会按顺序调用thread开头的函数并行计算。
        // 主线程则会调用子线程对应函数。由于所有线程都会前后使用Barrier可以保证两者同步。
        // 主线程在子线程并行计算完后进行无法并行的处理代码。例如updateAction
        void threadController(std::set<Vehicle *> &vehicles, 
                              std::vector<Road *> &roads,
                              std::vector<Intersection *> &intersections,
                              std::vector<Drivable *> &drivables);

        void threadPlanRoute(const std::vector<Road *> &roads); // 检查车辆是否计划变道

        void threadGetAction(std::set<Vehicle *> &vehicles); // 每辆车执行vehicleControl

        // 对于每辆在跑的车，处理被拦住的车已经消失的情况，然后更新车辆状态，
        void threadUpdateAction(std::set<Vehicle *> &vehicles);

        void threadUpdateLeaderAndGap(const std::vector<Drivable *> &drivables); // 更新车辆前车和距离

        // 处理要移除的车辆，即驶到新路或已结束行程
        void threadUpdateLocation(const std::vector<Drivable *> &drivables);

        // 检查路口车辆是否有压到或即将开到交点的，有的话Cross.notify
        void threadNotifyCross(const std::vector<Intersection *> &intersections);

        // 重新整顿Segment内容
        void threadInitSegments(const std::vector<Road *> &roads);

        // 多线程规划变道，准备变道加入laneChangeNotifyBuffer
        void threadPlanLaneChange(const std::set<Vehicle *> &vehicles);

    public:
        std::mt19937 rnd;

        Engine(const std::string &configFile, int threadNum);

        double getInterval() const { return interval; }

        bool hasLaneChange() const { return laneChange; }

        bool loadConfig(const std::string &configFile);

        void notifyCross(); // sync

        // 模拟器执行下一步。流程如下：
        // - flow生成新的车辆
        // - 规划车辆轨迹
        // - 检查等待车辆是否能够进入车道
        // - 如果有变道操作则
        //   + 重置segments的内容
        //   + 进行变道，生成影子车辆
        //   + 更新车辆前车和距离
        // - 检查交点，并设置阻拦车辆
        // - 添加开到新的drivable的车辆
        // - 更新车辆操作
        // - 更新每辆车的前置车辆和距离
        // - 如果不由算法控制红绿灯则每个路口自动变灯
        // - 如果保存回放存log
        void nextStep();

        bool checkPriority(int priority); // 检查priority是否存在

        void pushVehicle(Vehicle *const vehicle, bool pushToDrivable = true);

        void setLogFile(const std::string &jsonFile, const std::string &logFile);

        void initSegments(); // sync

        ~Engine();

        // RL related api

        void pushVehicle(const std::map<std::string, double> &info, 
            const std::vector<std::string> &roads); // 主动添加一辆车。info中包含车辆数据

        size_t getVehicleCount() const;

        std::vector<std::string> getVehicles(bool includeWaiting = false) const;

        std::map<std::string, int> getLaneVehicleCount() const;

        std::map<std::string, int> getLaneWaitingVehicleCount() const; // 速度小于0.1m/s

        std::map<std::string, std::vector<std::string>> getLaneVehicles();

        std::map<std::string, double> getVehicleSpeed() const;

        std::map<std::string, double> getVehicleDistance() const; // 在现在路上开了多少距离

        std::string getLeader(const std::string &vehicleId) const;

        double getCurrentTime() const;

        double getAverageTravelTime() const; // 包含已结束行程(结束停止计时)和正在开的车辆(不论何时出发)

        void setTrafficLightPhase(const std::string &id, int phaseIndex);

        void setReplayLogFile(const std::string &logFile);

        void setSaveReplay(bool open);

        void setVehicleSpeed(const std::string &id, double speed);

        void setRandomSeed(int seed) { rnd.seed(seed); }
        
        void reset(bool resetRnd = false);

        // archive
        void load(const Archive &archive) { archive.resume(*this); }
        Archive snapshot() { return Archive(*this); }
        void loadFromFile(const char *fileName);
    };

}

#endif //CITYFLOW_ENGINE_H
