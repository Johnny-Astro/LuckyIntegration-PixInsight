#include "LuckyIntegrationParameters.h"

namespace pcl
{

LIRoutine* TheLIRoutineParameter = nullptr;
LIInputPath* TheLIInputPathParameter = nullptr;
LIApproxFWHM* TheLIApproxFWHMParameter = nullptr;
LIMinPeak* TheLIMinPeakParameter = nullptr;
LISaturationThreshold* TheLISaturationThresholdParameter = nullptr;
LIMasterDarkPath* TheLIMasterDarkPathParameter = nullptr;
LIMasterFlatPath* TheLIMasterFlatPathParameter = nullptr;
LIPedestal* TheLIPedestalParameter = nullptr;
LIEnableDigitalAO* TheLIEnableDigitalAOParameter = nullptr;
LIStarSizeRejectionThreshold* TheLIStarSizeRejectionThresholdParameter = nullptr;
LIStarMovementRejectionThreshold* TheLIStarMovementRejectionThresholdParameter = nullptr;
LIInterpolation* TheLIInterpolationParameter = nullptr;
LIFramePercentage* TheLIFramePercentageParameter = nullptr;
LIRegistrationOnly* TheLIRegistrationOnlyParameter = nullptr;
LIRegistrationOutputPath* TheLIRegistrationOutputPathParameter = nullptr;

LIRoutine::LIRoutine(MetaProcess* P) : MetaEnumeration(P)
{
    TheLIRoutineParameter = this;
}

IsoString LIRoutine::Id() const
{
    return "routine";
}

size_type LIRoutine::NumberOfElements() const
{
    return NumberOfRoutines;
}

IsoString LIRoutine::ElementId(size_type i) const
{
    switch (i)
    {
    default:
    case StarDetectionPreview:      return "StarDetectionPreview";
    case StarDetectionAlignment:    return "StarDetectionAlignment";
    case ImageIntegration:          return "ImageIntegration";
    }
}

int LIRoutine::ElementValue(size_type i) const
{
    return int(i);
}

size_type LIRoutine::DefaultValueIndex() const
{
    return size_type(Default);
}

LIInputPath::LIInputPath(MetaProcess* P) : MetaString(P)
{
    TheLIInputPathParameter = this;
}

IsoString LIInputPath::Id() const
{
    return "inputPath";
}

LIApproxFWHM::LIApproxFWHM(MetaProcess* P) : MetaFloat(P)
{
    TheLIApproxFWHMParameter = this;
}

IsoString LIApproxFWHM::Id() const
{
    return "approxFWHM";
}

int LIApproxFWHM::Precision() const
{
    return 1;
}

double LIApproxFWHM::MinimumValue() const
{
    return 1.0;
}

double LIApproxFWHM::MaximumValue() const
{
    return 20.0;
}

double LIApproxFWHM::DefaultValue() const
{
    return 5.0;
}

LIMinPeak::LIMinPeak(MetaProcess* P) : MetaFloat(P)
{
    TheLIMinPeakParameter = this;
}

IsoString LIMinPeak::Id() const
{
    return "minPeak";
}

int LIMinPeak::Precision() const
{
    return 3;
}

double LIMinPeak::MinimumValue() const
{
    return 0.001;
}

double LIMinPeak::MaximumValue() const
{
    return 0.5;
}

double LIMinPeak::DefaultValue() const
{
    return 0.1;
}

LISaturationThreshold::LISaturationThreshold(MetaProcess* P) : MetaFloat(P)
{
    TheLISaturationThresholdParameter = this;
}

IsoString LISaturationThreshold::Id() const
{
    return "saturationThreshold";
}

int LISaturationThreshold::Precision() const
{
    return 2;
}

double LISaturationThreshold::MinimumValue() const
{
    return 0.1;
}

double LISaturationThreshold::MaximumValue() const
{
    return 1.0;
}

double LISaturationThreshold::DefaultValue() const
{
    return 0.85;
}

LIMasterDarkPath::LIMasterDarkPath(MetaProcess* P) : MetaString(P)
{
    TheLIMasterDarkPathParameter = this;
}

IsoString LIMasterDarkPath::Id() const
{
    return "masterDarkPath";
}

LIMasterFlatPath::LIMasterFlatPath(MetaProcess* P) : MetaString(P)
{
    TheLIMasterFlatPathParameter = this;
}

IsoString LIMasterFlatPath::Id() const
{
    return "masterFlatPath";
}

LIPedestal::LIPedestal(MetaProcess* P) : MetaFloat(P)
{
    TheLIPedestalParameter = this;
}

IsoString LIPedestal::Id() const
{
    return "pedestal";
}

int LIPedestal::Precision() const
{
    return 4;
}

double LIPedestal::MinimumValue() const
{
    return 0.0;
}

double LIPedestal::MaximumValue() const
{
    return 0.01;
}

double LIPedestal::DefaultValue() const
{
    return 0.001;
}

LIEnableDigitalAO::LIEnableDigitalAO(MetaProcess* P) : MetaBoolean(P)
{
    TheLIEnableDigitalAOParameter = this;
}

IsoString LIEnableDigitalAO::Id() const
{
    return "enableDigitalAO";
}

bool LIEnableDigitalAO::DefaultValue() const
{
    return true;
}

LIStarSizeRejectionThreshold::LIStarSizeRejectionThreshold(MetaProcess* P) : MetaFloat(P)
{
    TheLIStarSizeRejectionThresholdParameter = this;
}

IsoString LIStarSizeRejectionThreshold::Id() const
{
    return "starSizeRejectionThreshold";
}

int LIStarSizeRejectionThreshold::Precision() const
{
    return 1;
}

double LIStarSizeRejectionThreshold::MinimumValue() const
{
    return 1.0;
}

double LIStarSizeRejectionThreshold::MaximumValue() const
{
    return 30.0;
}

double LIStarSizeRejectionThreshold::DefaultValue() const
{
    return 10.0;
}

LIStarMovementRejectionThreshold::LIStarMovementRejectionThreshold(MetaProcess* P) : MetaFloat(P)
{
    TheLIStarMovementRejectionThresholdParameter = this;
}

IsoString LIStarMovementRejectionThreshold::Id() const
{
    return "starMovementRejectionThreshold";
}

int LIStarMovementRejectionThreshold::Precision() const
{
    return 1;
}

double LIStarMovementRejectionThreshold::MinimumValue() const
{
    return 1.0;
}

double LIStarMovementRejectionThreshold::MaximumValue() const
{
    return 100.0;
}

double LIStarMovementRejectionThreshold::DefaultValue() const
{
    return 3.0;
}

LIInterpolation::LIInterpolation(MetaProcess* P) : MetaEnumeration(P)
{
    TheLIInterpolationParameter = this;
}

IsoString LIInterpolation::Id() const
{
    return "interpolation";
}

size_type LIInterpolation::NumberOfElements() const
{
    return NumberOfInterpolations;
}

IsoString LIInterpolation::ElementId(size_type i) const
{
    switch (i)
    {
    default:
    case Nearest:   return "Nearest";
    case Bilinear:  return "Bilinear";
    case Lanczos3:  return "Lanczos3";
    }
}

int LIInterpolation::ElementValue(size_type i) const
{
    return int(i);
}

size_type LIInterpolation::DefaultValueIndex() const
{
    return size_type(Default);
}

LIFramePercentage::LIFramePercentage(MetaProcess* P) : MetaFloat(P)
{
    TheLIFramePercentageParameter = this;
}

IsoString LIFramePercentage::Id() const
{
    return "framePercentage";
}

int LIFramePercentage::Precision() const
{
    return 1;
}

double LIFramePercentage::MinimumValue() const
{
    return 0.0;
}

double LIFramePercentage::MaximumValue() const
{
    return 100.0;
}

double LIFramePercentage::DefaultValue() const
{
    return 100.0;
}

LIRegistrationOnly::LIRegistrationOnly(MetaProcess* P) : MetaBoolean(P)
{
    TheLIRegistrationOnlyParameter = this;
}

IsoString LIRegistrationOnly::Id() const
{
    return "registrationOnly";
}

bool LIRegistrationOnly::DefaultValue() const
{
    return true;
}

LIRegistrationOutputPath::LIRegistrationOutputPath(MetaProcess* P) : MetaString(P)
{
    TheLIRegistrationOutputPathParameter = this;
}

IsoString LIRegistrationOutputPath::Id() const
{
    return "registrationOutputPath";
}

}	// namespace pcl