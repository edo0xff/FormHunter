#pragma once

#include "Packet.h"
#include <map>
#include <vector>
#include <string>

class Metric
{
private:
    typedef std::pair<std::string, uint32_t> HostRequests;

    uint32_t totalHttpRequests;
    std::map<std::string, uint32_t> requestsPerHostname;
    std::vector<std::string> formInputFilters;

    // This will help us to build the histogram
    HostRequests popular;

    std::string getHistogramBar(uint32_t reqCount) const;
    std::string getHistogram() const;

    bool http_flag;

public:

    inline const bool getHttpFlag() const { return http_flag; }

    Metric();
    virtual ~Metric();

    void setHttpFlag(bool value);
    void setFormInputFilters(std::vector<std::string> p_formInputFilters);
    bool formInputMatchesFilters(std::string formInputString);

    void consume(pcpp::Packet &pkt);
    void writeToFile(const std::string &path) const;
};