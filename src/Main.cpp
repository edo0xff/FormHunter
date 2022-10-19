#include <cstdlib>
#include <string>
#include <memory>

#include "tclap/CmdLine.h"
#include "spdlog/spdlog.h"
#include "HttpRecorder.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#pragma comment(lib, "iphlpapi.lib")
#endif

int main(int argc, char *argv[])
{
    TCLAP::CmdLine cmd("Small C++ command line tool for HTTP Forms interception", ' ', "0.0.1");
    TCLAP::SwitchArg listIfacesArg("l", "list-ifaces", "show available network ifaces", false);
    TCLAP::ValueArg<std::string> inetArg("i", "interface", "network interface to intercept", false, "eth0", "string");
    TCLAP::ValueArg<uint32_t> timeLapseArg("t", "time-lapse", "duration in seconds interception will last", false, 10, "non-negative integer");
    TCLAP::ValueArg<std::string> outFileArg("o", "output", "output filepath where results will be written", false, "./output.txt", "string");
    TCLAP::ValueArg<std::string> filtersArg("f", "filters", "form inputs that will be filtered (comma separated)", false, "", "string");

    cmd.add(listIfacesArg);
    cmd.add(inetArg);
    cmd.add(timeLapseArg);
    cmd.add(outFileArg);
    cmd.add(filtersArg);
    cmd.parse(argc, argv);

    if (listIfacesArg.getValue())
    {
        HttpRecorder::listNetworkIfaces();

        return EXIT_SUCCESS;
    }

    const std::string networkAdapter = inetArg.getValue();
    const uint32_t timeLapse = timeLapseArg.getValue();
    const std::string outputFilePath = outFileArg.getValue();
    const std::string filtersString = filtersArg.getValue();

    std::unique_ptr<HttpRecorder> httpRecorder;

    try
    {
        httpRecorder = std::make_unique<HttpRecorder>(networkAdapter, timeLapse);
    }
    catch (std::exception &e)
    {
        spdlog::error("unable to initialize http recorder: {}", e.what());
        return EXIT_FAILURE;
    }

    if (filtersString != "")
        httpRecorder->setFormInputFilters(filtersString);

    httpRecorder->start();
    httpRecorder->writeToFile(outputFilePath);

    return EXIT_SUCCESS;
}
