#ifndef CITYFLOW_FLOW_H
#define CITYFLOW_FLOW_H

#include "vehicle/vehicle.h"
#include "flow/route.h"
#include "utility/config.h"

namespace CityFlow {
    class Engine;

    struct VehicleInfo;

    class Flow {
        friend class Archive;
    private:
        VehicleInfo vehicleTemplate;
        std::shared_ptr<const Route> route;
        double interval;
        double nowTime = 0;
        double currentTime = 0;
        double startTime = 0;
        double endTime = -1;
        int cnt = 0;
        Engine *engine;
        std::string id;
        bool valid = true;

    public:
        Flow(const VehicleInfo &vehicleTemplate, double timeInterval,
            Engine *engine, double startTime, double endTime, const std::string &id) 
            : vehicleTemplate(vehicleTemplate), interval(timeInterval),
              startTime(startTime), endTime(endTime), engine(engine), id(id) {
            assert(timeInterval > 0);
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
