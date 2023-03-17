#ifndef __LuckyIntegrationProcess_h
#define __LuckyIntegrationProcess_h

#include <pcl/MetaProcess.h>

namespace pcl
{

class LuckyIntegrationProcess : public MetaProcess
{
public:
    LuckyIntegrationProcess();

    IsoString Id() const override;
    IsoString Category() const override;
    uint32 Version() const override;
    String Description() const override;
    IsoString IconImageSVG() const override;
    ProcessInterface* DefaultInterface() const override;
    ProcessImplementation* Create() const override;
    ProcessImplementation* Clone(const ProcessImplementation&) const override;
    bool NeedsValidation() const override;
    bool CanProcessCommandLines() const override;
};

PCL_BEGIN_LOCAL
extern LuckyIntegrationProcess* TheLuckyIntegrationProcess;
PCL_END_LOCAL

}	// namespace pcl

#endif	// __LuckyIntegrationProcess_h
