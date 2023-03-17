#ifndef __LuckyIntegrationParameters_h
#define __LuckyIntegrationParameters_h

#include <pcl/MetaParameter.h>

namespace pcl
{

PCL_BEGIN_LOCAL

class LIRoutine : public MetaEnumeration
{
public:
    enum {
        StarDetectionPreview,
        StarDetectionAlignment,
        ImageIntegration,
        NumberOfRoutines,
        Default = StarDetectionPreview
    };

    LIRoutine(MetaProcess*);

    IsoString Id() const override;
    size_type NumberOfElements() const override;
    IsoString ElementId(size_type) const override;
    int ElementValue(size_type) const override;
    size_type DefaultValueIndex() const override;
};

extern LIRoutine* TheLIRoutineParameter;

class LIInputPath : public MetaString
{
public:
    LIInputPath(MetaProcess*);

    IsoString Id() const override;
};

extern LIInputPath* TheLIInputPathParameter;

class LIApproxFWHM : public MetaFloat
{
public:
    LIApproxFWHM(MetaProcess*);

    IsoString Id() const override;
    int Precision() const override;
    double MinimumValue() const override;
    double MaximumValue() const override;
    double DefaultValue() const override;
};

extern LIApproxFWHM* TheLIApproxFWHMParameter;

class LIMinPeak : public MetaFloat
{
public:
    LIMinPeak(MetaProcess*);

    IsoString Id() const override;
    int Precision() const override;
    double MinimumValue() const override;
    double MaximumValue() const override;
    double DefaultValue() const override;
};

extern LIMinPeak* TheLIMinPeakParameter;

class LISaturationThreshold : public MetaFloat
{
public:
    LISaturationThreshold(MetaProcess*);

    IsoString Id() const override;
    int Precision() const override;
    double MinimumValue() const override;
    double MaximumValue() const override;
    double DefaultValue() const override;
};

extern LISaturationThreshold* TheLISaturationThresholdParameter;

class LIMasterDarkPath : public MetaString
{
public:
    LIMasterDarkPath(MetaProcess*);

    IsoString Id() const override;
};

extern LIMasterDarkPath* TheLIMasterDarkPathParameter;

class LIMasterFlatPath : public MetaString
{
public:
    LIMasterFlatPath(MetaProcess*);

    IsoString Id() const override;
};

extern LIMasterFlatPath* TheLIMasterFlatPathParameter;

class LIPedestal : public MetaFloat
{
public:
    LIPedestal(MetaProcess*);

    IsoString Id() const override;
    int Precision() const override;
    double MinimumValue() const override;
    double MaximumValue() const override;
    double DefaultValue() const override;
};

extern LIPedestal* TheLIPedestalParameter;

class LIEnableDigitalAO : public MetaBoolean
{
public:
    LIEnableDigitalAO(MetaProcess*);

    IsoString Id() const override;
    bool DefaultValue() const override;
};

extern LIEnableDigitalAO* TheLIEnableDigitalAOParameter;

class LIStarSizeRejectionThreshold : public MetaFloat
{
public:
    LIStarSizeRejectionThreshold(MetaProcess*);

    IsoString Id() const override;
    int Precision() const override;
    double MinimumValue() const override;
    double MaximumValue() const override;
    double DefaultValue() const override;
};

extern LIStarSizeRejectionThreshold* TheLIStarSizeRejectionThresholdParameter;

class LIStarMovementRejectionThreshold : public MetaFloat
{
public:
    LIStarMovementRejectionThreshold(MetaProcess*);

    IsoString Id() const override;
    int Precision() const override;
    double MinimumValue() const override;
    double MaximumValue() const override;
    double DefaultValue() const override;
};

extern LIStarMovementRejectionThreshold* TheLIStarMovementRejectionThresholdParameter;

class LIInterpolation : public MetaEnumeration
{
public:
    enum {
        Nearest,
        Bilinear,
        Lanczos3,
        NumberOfInterpolations,
        Default = Bilinear
    };

    LIInterpolation(MetaProcess*);

    IsoString Id() const override;
    size_type NumberOfElements() const override;
    IsoString ElementId(size_type) const override;
    int ElementValue(size_type) const override;
    size_type DefaultValueIndex() const override;
};

extern LIInterpolation* TheLIInterpolationParameter;

class LIFramePercentage : public MetaFloat
{
public:
    LIFramePercentage(MetaProcess*);

    IsoString Id() const override;
    int Precision() const override;
    double MinimumValue() const override;
    double MaximumValue() const override;
    double DefaultValue() const override;
};

extern LIFramePercentage* TheLIFramePercentageParameter;

class LIRegistrationOnly : public MetaBoolean
{
public:
    LIRegistrationOnly(MetaProcess*);

    IsoString Id() const override;
    bool DefaultValue() const override;
};

extern LIRegistrationOnly* TheLIRegistrationOnlyParameter;

class LIRegistrationOutputPath : public MetaString
{
public:
    LIRegistrationOutputPath(MetaProcess*);

    IsoString Id() const override;
};

extern LIRegistrationOutputPath* TheLIRegistrationOutputPathParameter;

PCL_END_LOCAL

}	// namespace pcl

#endif	// __LuckyIntegrationParameters_h