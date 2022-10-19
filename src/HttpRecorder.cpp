#include "HttpRecorder.hpp"

#include <iostream>
#include <fstream>
#include <exception>
#include <vector>
#include "tabulate/table.hpp"
#include "spdlog/spdlog.h" // for logging
#include "SystemUtils.h"   // for pcpp::multiPlatformSleep
#include "HttpLayer.h"
#include "PayloadLayer.h"

static std::string urlDecode(std::string SRC);
static std::vector<std::string> stringSplit(std::string str, std::string delimiter);
static void onPacketArrives(pcpp::RawPacket *packet, pcpp::PcapLiveDevice *dev, void *cookie);

HttpRecorder::HttpRecorder(const std::string &networkInterfaceName, const uint32_t lapseInSeconds) : m_lapseInSeconds(lapseInSeconds)
{
    device = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByName(networkInterfaceName);

    if (device == NULL)
    {
        spdlog::error("unable to find network interface \"{}\"", networkInterfaceName);
        throw std::runtime_error("unable to find network interface");
    }

    if (!device->open())
    {
        spdlog::error("unable to open network device. You may run this program as root");
        throw std::runtime_error("unable to open network device");
    }
}

HttpRecorder::~HttpRecorder()
{
    device->stopCapture();
    device->close();
}

void HttpRecorder::listNetworkIfaces()
{
    spdlog::info("listing available network ifaces");

    tabulate::Table table;
    table.add_row({"Name", "MAC", "IPv4"});

    for (auto const &device : pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDevicesList())
    {
        table.add_row({device->getName(), device->getMacAddress().toString(), device->getIPv4Address().toString()});
    }

    std::cout << std::endl << table.str() << std::endl;
}

void HttpRecorder::start()
{
    spdlog::info("sniffing started");
    
    pcpp::ProtoFilter tcpFilter(pcpp::TCP); // HTTP No esta soportado aqui chavo, HTTP va sobre TCP
    device->setFilter(tcpFilter);

    device->startCapture(onPacketArrives, &metric);

    pcpp::multiPlatformSleep(m_lapseInSeconds);
    spdlog::info("done");
}

void HttpRecorder::setFormInputFilters(const std::string formInputFiltersString)
{
    metric.setFormInputFilters(stringSplit(formInputFiltersString, ","));
}

static std::string urlDecode(std::string source)
{
    std::string result;
    
    for (size_t i=0; i < source.length(); i++) 
    {
        if (source[i] == '%') 
        {
            uint64_t decimal = std::strtoul(source.substr(i+1, 2).c_str(), nullptr, 16);
            result += static_cast<char>(decimal);
            i = i + 2;
        }
        else 
        {
            result += source[i];
        }
    }

    return result;
}

static std::vector<std::string> stringSplit(std::string str, std::string delimiter)
{
    std::string token;
    size_t position = 0;
    std::vector<std::string> result;

    while ((position = str.find(delimiter)) != std::string::npos)
    {
        token = str.substr(0, position);
        result.push_back(token);
        str.erase(0, position + delimiter.length());
    }

    return result;
}

static void onPacketArrives(pcpp::RawPacket *packet, pcpp::PcapLiveDevice *dev, void *cookie)
{
    Metric *metric = (Metric *)cookie;

    pcpp::Packet parsedPacket(packet);
    pcpp::HttpRequestLayer* http_layer = parsedPacket.getLayerOfType<pcpp::HttpRequestLayer>();

    if (http_layer != NULL)
    {
        if (http_layer->toString().find("POST") != std::string::npos)
        {
            std::cout << std::endl << "== REQUEST ==" << std::endl;
            std::cout << http_layer->toString() << std::endl;

            pcpp::HeaderField* header = http_layer->getFirstField();

            for (;;)
            {
                if (header->getFieldName() != "")
                    std::cout << header->getFieldName() << ": " << header->getFieldValue() << std::endl;
                
                header = http_layer->getNextField(header);
                if (header == NULL) break;
            }

            char* payload = (char *)http_layer->getLayerPayload();
            payload[http_layer->getLayerPayloadSize()] = 0;

            std::cout << std::endl <<  "== PAYLOAD (" << http_layer->getLayerPayloadSize() << ") ==" << std::endl;

            for (std::string &line : stringSplit(payload, "&"))
            {
                line = urlDecode(line);

                if (metric->formInputMatchesFilters(line))
                    std::cout << line << std::endl;
            }            

            std::cout << std::endl;
            
            metric->consume(parsedPacket);
        }
    } 
    
    else
    {
        pcpp::PayloadLayer* payload_layer = parsedPacket.getLayerOfType<pcpp::PayloadLayer>();

        if (payload_layer != NULL)
        {
            char *payload = (char *)payload_layer->getPayload();
            payload[payload_layer->getPayloadLen()] = 0;

            std::string payload_string(payload);

            if (payload_string.find("POST /") != std::string::npos)
            {
                metric->setHttpFlag(true);
                std::cout << std::endl << "== REQUEST ==" << std::endl;
                std::cout << payload;
            } 
            
            else if (metric->getHttpFlag()) 
            {
                metric->setHttpFlag(false);
                std::cout << "== PAYLOAD (" << payload_layer->getPayloadLen() << ") ==" << std::endl;

                for (std::string &line : stringSplit(payload, "&"))
                {
                    line = urlDecode(line);

                    if (metric->formInputMatchesFilters(line))
                        std::cout << line << std::endl;
                }

                std::cout << std::endl;
            }

            metric->consume(parsedPacket);
        }
    }
}