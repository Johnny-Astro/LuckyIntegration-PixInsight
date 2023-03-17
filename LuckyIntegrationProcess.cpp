#include <pcl/Console.h>
#include <pcl/Arguments.h>
#include <pcl/View.h>
#include <pcl/Exception.h>

#include "LuckyIntegrationProcess.h"
#include "LuckyIntegrationParameters.h"
#include "LuckyIntegrationInstance.h"
#include "LuckyIntegrationInterface.h"

namespace pcl
{

LuckyIntegrationProcess* TheLuckyIntegrationProcess = nullptr;

LuckyIntegrationProcess::LuckyIntegrationProcess()
{
    TheLuckyIntegrationProcess = this;

    // Instantiate process parameters
    new LIRoutine(this);
    new LIInputPath(this);
    new LIApproxFWHM(this);
    new LIMinPeak(this);
    new LISaturationThreshold(this);
    new LIMasterDarkPath(this);
    new LIMasterFlatPath(this);
    new LIPedestal(this);
    new LIEnableDigitalAO(this);
    new LIStarSizeRejectionThreshold(this);
    new LIStarMovementRejectionThreshold(this);
    new LIFramePercentage(this);
    new LIInterpolation(this);
    new LIRegistrationOnly(this);
    new LIRegistrationOutputPath(this);
}

IsoString LuckyIntegrationProcess::Id() const
{
    return "LuckyIntegration";
}

IsoString LuckyIntegrationProcess::Category() const
{
    return "ImageIntegration,Preprocessing";
}

// ----------------------------------------------------------------------------

uint32 LuckyIntegrationProcess::Version() const
{
    return 0x100;
}

// ----------------------------------------------------------------------------

String LuckyIntegrationProcess::Description() const
{
    return "";
}

// ----------------------------------------------------------------------------

IsoString LuckyIntegrationProcess::IconImageSVG() const
{
    return "<svg width=\"512\" height=\"512\" xmlns=\"http://www.w3.org/2000/svg\">"
           "<g id = \"Layer_1\">"
           "<title>LuckyIntegration</title>"
           "<text transform = \"matrix(1.191 0 0 1.36283 -52.7099 -98.671)\" stroke=\"#000\" font-style=\"normal\" font-weight=\"normal\" xml:space=\"preserve\" text-anchor=\"start\" font-family=\"'Open Sans'\" font-size=\"120\" id=\"svg_1\" y=\"293.53247\" x=\"63.48111\" stroke-width=\"0\" fill=\"#ff0000\">LuckyIntegration</text>"
           "</g>"
           "</svg>";
}
// ----------------------------------------------------------------------------

ProcessInterface* LuckyIntegrationProcess::DefaultInterface() const
{
    return TheLuckyIntegrationInterface;
}
// ----------------------------------------------------------------------------

ProcessImplementation* LuckyIntegrationProcess::Create() const
{
    return new LuckyIntegrationInstance(this);
}

// ----------------------------------------------------------------------------

ProcessImplementation* LuckyIntegrationProcess::Clone(const ProcessImplementation& p) const
{
    const LuckyIntegrationInstance* instPtr = dynamic_cast<const LuckyIntegrationInstance*>(&p);
    return (instPtr != 0) ? new LuckyIntegrationInstance(*instPtr) : 0;
}

// ----------------------------------------------------------------------------

bool LuckyIntegrationProcess::NeedsValidation() const
{
    return false;
}

// ----------------------------------------------------------------------------

bool LuckyIntegrationProcess::CanProcessCommandLines() const
{
    return false;
}

}	// namespace pcl
