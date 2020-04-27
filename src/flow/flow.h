#ifndef CITYFLOW_FLOW_H
#define CITYFLOW_FLOW_H

#include <iostream>

#include "vehicle/vehicle.h"
#include "flow/route.h"

namespace CityFlow {
    class Engine;

    struct VehicleInfo;

    class Flow {
        friend class Archive;
    private:
        VehicleInfo vehicleTemplate;
        std::shared_ptr<const Route> route;
        double interval;
        double nowTime = 0; // 距离上次产生车辆时间。最开始初始化为interval，到时刻以后每经过interval时间就产生车辆。
        double currentTime = 0; // 真实模拟时间
        int startTime = 0;
        int endTime = -1;
        int cnt = 0; // 计数器，nextStep时会+1，保持生成车辆名称不重复
        Engine *engine;
        std::string id;
        bool valid = true; // 路径是否合法

    public:
        Flow(const VehicleInfo &vehicleTemplate, double timeInterval,
            Engine *engine, int startTime, int endTime, const std::string &id) 
            : vehicleTemplate(vehicleTemplate), interval(timeInterval),
              startTime(startTime), endTime(endTime), engine(engine), id(id) {
            assert(timeInterval >= 1 || (startTime == endTime));
            nowTime = interval;
        }

        void nextStep(double timeInterval);

        std::string getId() const;

        bool isValid() const { return this->valid; }

        void setValid(const bool valid) {
            if (this->valid && !valid)
                std::cerr << "[warning] Invalid route '" << id << "'. Omitted by default." << std::endl;
            this->valid = valid;
        }

        void reset();

    };
}

#endif //CITYFLOW_FLOW_H
