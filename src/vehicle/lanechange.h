#ifndef CITYFLOW_LANECHANGE_H
#define CITYFLOW_LANECHANGE_H

#include "roadnet/roadnet.h"

#include <memory>

namespace CityFlow {

    class Vehicle;
    class Lane;

    class LaneChange {
        friend class Vehicle;
        friend class Archive;
    //The interface of lane changing
     protected:
        struct Signal{
            int urgency;
            int direction; // -1 for left , 1 for right, 0 for unchanged
            Lane * target;
            Vehicle * source;
            int response = 0;
            double extraSpace = 0;
        };

        int lastDir; // 上次变道的方向

        std::shared_ptr<Signal> signalRecv;
        std::shared_ptr<Signal> signalSend;

        Vehicle * vehicle;
        Vehicle * targetLeader = nullptr; // 变道后开在该车前面
        Vehicle * targetFollower = nullptr; // 变道后跟在该车后面

        double leaderGap;
        double followerGap;
        double waitingTime = 0; // 等待时间。在yieldSpeed中累计。

        bool changing = false; // insertShadow中变成true，表示开始转弯
        bool finished = false;
        double lastChangeTime = 0; // 上次变道时间

        static constexpr double coolingTime = 3;

    public:
        LaneChange(Vehicle * vehicle, const LaneChange &other); // 以该车生成LaneChange，并从other抄配置

        explicit LaneChange(Vehicle * vehicle) : vehicle(vehicle) {};

        virtual ~LaneChange() = default;

        void updateLeaderAndFollower(); // 更新前后车辆及相关距离

        Lane *getTarget() const;

        Vehicle *getTargetLeader() const {
            return targetLeader;
        }

        Vehicle *getTargetFollower() const {
            return targetFollower;
        }

        double gapBefore() const ; // followerGap

        double gapAfter() const ;  // leaderGap

        void insertShadow(Vehicle *shadow) ; // 添加一个虚拟车辆到地图中，除了改变路段其他照抄

        virtual double safeGapBefore() const = 0; // 车辆与变道后前车需要保持的最小距离
        virtual double safeGapAfter() const = 0; // 车辆与变道后后车需要保持的最小距离

        // 当要求该车变道时，做成新的signalSend，保存相关变道信息
        virtual void makeSignal(double interval) { if (signalSend) signalSend->direction = getDirection(); };

        bool planChange() const; // 是否准备转弯

        bool canChange() const { return signalSend && !signalRecv; } // 当没有收到信号且发出了信号就能变道

        bool isGapValid() const { return gapAfter() >= safeGapAfter() && gapBefore() >= safeGapBefore(); }

        void finishChanging();

        void abortChanging();

        // 变道相关车的最高行驶速度
        // interval: 距离上次调用的时间
        virtual double yieldSpeed(double interval) = 0;

        virtual void sendSignal() = 0; // 发送信号，将信号发给哪些车辆

        int getDirection();

        void clearSignal(); // 清除信号，除非正在变道

        bool hasFinished() const { return this->finished; }

    };

    class SimpleLaneChange : public LaneChange {
    private:
        double estimateGap(const Lane *lane) const; // 计算变道过去剩余空间大小
    public:
        explicit SimpleLaneChange(Vehicle * vehicle) : LaneChange(vehicle) {};
        explicit SimpleLaneChange(Vehicle * vehicle, const LaneChange &other) : LaneChange(vehicle, other) {};

        // 生成转弯信号。如果在LaneLink上就生成不转弯信号。
        void makeSignal(double interval) override;
        void sendSignal() override; // 向变道后的前后车发送变道信号

        // 收到其他车变道的人的预计速度。前车最快往前。后车保持不撞速度，或者实在会撞就不管了最快往前
        double yieldSpeed(double interval) override;

        double safeGapBefore() const override; // 后车紧急刹车距离

        double safeGapAfter() const override; // 自己紧急刹车距离

    };
}

#endif //CITYFLOW_LANECHANGE_H
