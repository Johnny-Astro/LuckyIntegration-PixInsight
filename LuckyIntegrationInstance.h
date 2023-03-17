#ifndef __LuckyIntegrationInstance_h
#define __LuckyIntegrationInstance_h

#include <pcl/ProcessImplementation.h>
#include <pcl/MetaParameter.h> // pcl_enum
#include <pcl/Mutex.h>

#include "NativeImage.h"

namespace pcl
{

struct Star
{
    int id;
    float x;
    float y;
    float background;
    float peak;
    float sizeX;
    float sizeY;
};

class LuckyIntegrationInstance : public ProcessImplementation
{
public:
    LuckyIntegrationInstance(const MetaProcess*);
    LuckyIntegrationInstance(const LuckyIntegrationInstance&);

    void Assign(const ProcessImplementation&) override;
    bool IsHistoryUpdater(const View& v) const override;
    bool CanExecuteOn(const View&, String& whyNot) const override;
    bool CanExecuteGlobal(String& whyNot) const override;
    bool ExecuteGlobal() override;
    void* LockParameter(const MetaParameter*, size_type tableRow) override;
    bool AllocateParameter(size_type sizeOrLength, const MetaParameter*, size_type tableRow) override;
    size_type ParameterLength(const MetaParameter*, size_type tableRow) const override;

private:
    struct ImageItem
    {
        pcl_bool enabled = true;
        String   path;    // image file

        ImageItem() = default;
        ImageItem(const ImageItem&) = default;

        ImageItem(const String& a_path) : path(a_path)
        {
        }

        bool IsDefined() const
        {
            return !path.IsEmpty();
        }
    };

    pcl_enum p_routine;
    String p_inputPath;
    double p_approxFwhm;
    double p_minPeak;
    double p_saturationThreshold;
    ImageItem p_masterDark;
    ImageItem p_masterFlat;
    double p_pedestal;
    pcl_bool p_enableDigitalAO;
    double p_starSizeRejectionThreshold;
    double p_starMovementRejectionThreshold;
    pcl_enum p_interpolation;
    double p_framePercentage;
    pcl_bool p_registrationOnly;
    String p_registrationOutputPath;

    NativeImage m_masterDarkImage;
    NativeImage m_masterFlatImage;
    float m_masterFlatMean;
    bool m_hasDark;
    bool m_hasFlat;

    NativeImage m_backgroundImage;
    NativeImage m_starDetectionPreviewImage;
    Array<Array<Star>> m_starDetections;
    Mutex m_starDetectionLock;
    NativeImage m_starMovementImage;
    NativeImage m_integration;
    NativeImage m_weight;
    int m_numTotalImages;
    int m_numIntegratedImages;
    double m_averageProcessTimeMs;

    void doStarDetectionPreview();
    void doStarDetectionAlignment();
    void doImageIntegration();

    friend class LuckyIntegrationProcess;
    friend class LuckyIntegrationInterface;
    friend class ImageThread;
    friend class StarDetectionThread;
    friend class ImageIntegrationThread;
};

}	// namespace pcl

#endif	// __LuckyIntegrationInstance_h
