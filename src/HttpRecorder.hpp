#pragma once

#include <string>
#include <map>

#include "Metric.hpp"
#include "PcapLiveDeviceList.h"

class HttpRecorder
{
private:
    uint32_t m_lapseInSeconds;
    Metric metric;

    pcpp::PcapLiveDevice *device;

public:
    HttpRecorder(const std::string &networkInterfaceName, const uint32_t lapseInSeconds);
    virtual ~HttpRecorder();

    static void listNetworkIfaces();

    void start();
    void setFormInputFilters(const std::string formInputFiltersString);

    inline void writeToFile(const std::string &path) const { metric.writeToFile(path); };
};