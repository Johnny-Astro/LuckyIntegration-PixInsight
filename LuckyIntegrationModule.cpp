#define MODULE_VERSION_MAJOR     1
#define MODULE_VERSION_MINOR     0
#define MODULE_VERSION_REVISION  0
#define MODULE_VERSION_BUILD     0
#define MODULE_VERSION_LANGUAGE  eng

#define MODULE_RELEASE_YEAR      2022
#define MODULE_RELEASE_MONTH     11
#define MODULE_RELEASE_DAY       26

#include "LuckyIntegrationModule.h"
#include "LuckyIntegrationProcess.h"
#include "LuckyIntegrationInterface.h"

namespace pcl
{

LuckyIntegrationModule::LuckyIntegrationModule()
{
}

const char* LuckyIntegrationModule::Version() const
{
    return PCL_MODULE_VERSION(MODULE_VERSION_MAJOR,
                              MODULE_VERSION_MINOR,
                              MODULE_VERSION_REVISION,
                              MODULE_VERSION_BUILD,
                              MODULE_VERSION_LANGUAGE);
}

IsoString LuckyIntegrationModule::Name() const
{
    return "LuckyIntegration";
}

String LuckyIntegrationModule::Description() const
{
    return "PixInsight LuckyIntegration Module";
}

String LuckyIntegrationModule::Company() const
{
    return "N/A";
}

String LuckyIntegrationModule::Author() const
{
    return "Johnny Qiu";
}

String LuckyIntegrationModule::Copyright() const
{
    return "Copyright (c) 2021 Johnny Qiu";
}

String LuckyIntegrationModule::TradeMarks() const
{
    return "JQ";
}

String LuckyIntegrationModule::OriginalFileName() const
{
#ifdef __PCL_LINUX
    return "LuckyIntegration-pxm.so";
#endif
#ifdef __PCL_FREEBSD
    return "LuckyIntegration-pxm.so";
#endif
#ifdef __PCL_MACOSX
    return "LuckyIntegration-pxm.dylib";
#endif
#ifdef __PCL_WINDOWS
    return "LuckyIntegration-pxm.dll";
#endif
}

void LuckyIntegrationModule::GetReleaseDate(int& year, int& month, int& day) const
{
    year = MODULE_RELEASE_YEAR;
    month = MODULE_RELEASE_MONTH;
    day = MODULE_RELEASE_DAY;
}

}   // namespace pcl

PCL_MODULE_EXPORT int InstallPixInsightModule(int mode)
{
    new pcl::LuckyIntegrationModule;

    if (mode == pcl::InstallMode::FullInstall) {
        new pcl::LuckyIntegrationProcess;
        new pcl::LuckyIntegrationInterface;
    }

    return 0;
}
