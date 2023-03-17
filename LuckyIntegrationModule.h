#ifndef __LuckyIntegrationModule_h
#define __LuckyIntegrationModule_h

#include <pcl/MetaModule.h>

namespace pcl
{

class LuckyIntegrationModule : public MetaModule
{
public:
    LuckyIntegrationModule();

    const char* Version() const override;
    IsoString Name() const override;
    String Description() const override;
    String Company() const override;
    String Author() const override;
    String Copyright() const override;
    String TradeMarks() const override;
    String OriginalFileName() const override;
    void GetReleaseDate(int& year, int& month, int& day) const override;
};

}   // namespace pcl

#endif  // __LuckyIntegrationModule_h
