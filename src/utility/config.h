#ifndef CITYFLOW_CONFIG_H
#define CITYFLOW_CONFIG_H

#include <deque>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <mutex>
#include <set>
#include <random>
#include <fstream>
#include <memory>
#include <list>
#include <map>
#include <queue>
#include <vector>
#include <cmath>
#include <typeinfo>
#include <stdexcept>
#include <utility>
#include <sstream>
#include <string>
#include <algorithm>
#include <limits>
#include <cstring>

//#include <log4cxx/basicconfigurator.h>
//#include <log4cxx/helpers/exception.h>
//#include <log4cxx/propertyconfigurator.h>
//using namespace log4cxx;

#ifdef GTEST_PUBLIC
    #define private public
    #define protected public
#endif

class GTestInjectSwitchClass {
public:
    bool Router_getFirstDrivable_random;
    bool Router_getFirstDrivable_random_notify;
    void clear() {
        memset(this, 0, sizeof (*this));
    }
};
extern GTestInjectSwitchClass GTestInjectSwitch;

/*
struct LOG4CXXLogClass {
    std::string filename;

    const std::string P1 = "log4j.rootLogger=";
    const std::string P2 = ",fa,ca\nlog4j.appender.ca=org.apache.log4j.ConsoleAppender\nlog4j.appender.ca.layout=org.apache.log4j.PatternLayout\nlog4j.appender.ca.layout.ConversionPattern=[%-5p][%d] : %m%n\nlog4j.appender.fa=org.apache.log4j.FileAppender\nlog4j.appender.fa.File=";
    const std::string P3 = "\nlog4j.appender.fa.layout=org.apache.log4j.PatternLayout\nlog4j.appender.fa.layout.ConversionPattern=[%-5p][%d] : %m%n";

    LoggerPtr lp;

    LOG4CXXLogClass() : lp(Logger::getRootLogger()) {}

    void init(const std::string &ifilename, const std::string &level = "ALL"){
        filename = ifilename;
        if (filename.size() == 0) return;
        std::string PP = P1 + "INFO" + P2 + filename + P3;
        srand(unsigned(time(NULL)));
        rand();
        char propfile[200];
        sprintf(propfile, "/tmp/%d", rand());
        FILE *f = fopen(propfile, "w");
        fprintf(f, "%s", PP.c_str());
        PropertyConfigurator::configureAndWatch(propfile);
    }

    void log(const std::string &data, const std::string &level = "DEBUG"){
        if (!filename.size()) return;
        if (level == "INFO"){
            LOG4CXX_INFO(lp, data);
        }
        else if (level == "ERROR"){
            LOG4CXX_ERROR(lp, data);
        }
        else if (level == "DEBUG"){
            LOG4CXX_DEBUG(lp, data);
        }
    }
};
*/

struct LogClass {
    std::string filename;
    std::ofstream o;

    std::mutex lock;

	static std::string getTime()
	{
		time_t timep;
		time (&timep);
		char tmp[64];
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep));
		return tmp;
	}

    void init(const std::string &ifilename, const std::string &level = "ALL"){
        filename = ifilename;
        if (level != "ALL"){
            throw "not support log level control now!";
        }
        if (!filename.size()) return;
        o = std::ofstream(filename);
    }

    void log(const std::string &data, std::string level = "DEBUG"){
        if (!filename.size()) return;
        std::lock_guard<std::mutex> guard(lock);
        for (; level.size() < 5; level += ' ');
        o << '[' << level << "] " << getTime() << ' ' << data << '\n' << std::flush;
	}

};

extern LogClass Log;

#define LOG(X) Log.log(X)
#define LOG2(X,Y) Log.log(X,Y)

namespace CityFlow {
    const int MAX_NUM_CARS_ON_SEGMENT = 10;
}

#endif //CITYFLOW_CONFIG_H
