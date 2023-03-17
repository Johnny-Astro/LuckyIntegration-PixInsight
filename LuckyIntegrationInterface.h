#ifndef __LuckyIntegrationInterface_h
#define __LuckyIntegrationInterface_h

#include <pcl/ComboBox.h>
#include <pcl/NumericControl.h>
#include <pcl/ProcessInterface.h>
#include <pcl/PushButton.h>
#include <pcl/SectionBar.h>
#include <pcl/Sizer.h>
#include <pcl/TreeBox.h>

#include "LuckyIntegrationInstance.h"

namespace pcl {

class LuckyIntegrationInterface : public ProcessInterface
{
public:
    LuckyIntegrationInterface();
    virtual ~LuckyIntegrationInterface();

    IsoString Id() const override;
    MetaProcess* Process() const override;
    IsoString IconImageSVG() const override;
    InterfaceFeatures Features() const override;
    void ResetInstance() override;
    bool Launch(const MetaProcess&, const ProcessImplementation*, bool& dynamic, unsigned& /*flags*/) override;
    ProcessImplementation* NewProcess() const override;
    bool ValidateProcess(const ProcessImplementation&, pcl::String& whyNot) const override;
    bool RequiresInstanceValidation() const override;
    bool ImportProcess(const ProcessImplementation&) override;
    void SaveSettings() const override;

private:

    LuckyIntegrationInstance m_instance;

    struct GUIData
    {
        GUIData(LuckyIntegrationInterface&);

        VerticalSizer   Global_Sizer;

        HorizontalSizer Routine_Sizer;
            Label           Routine_Lable;
            ComboBox        Routine_ComboBox;

        SectionBar      InputPath_SectionBar;
        Control         InputPath_Control;
        HorizontalSizer InputPath_Sizer;
            Label           InputPath_Label;
            Edit            InputPath_Edit;
            ToolButton      InputPath_ToolButton;
            ToolButton      InputPathClear_ToolButton;

        SectionBar      StarDetection_SectionBar;
        Control         StarDetection_Control;
        VerticalSizer   StarDetection_Sizer;
            NumericControl  ApproxFWHM_NumericControl;
            NumericControl  MinPeak_NumericControl;
            NumericControl  SaturationThreshold_NumericControl;

        SectionBar      Calibration_SectionBar;
        Control         Calibration_Control;
        VerticalSizer   Calibration_Sizer;
            HorizontalSizer     MasterDarkPath_Sizer;
                Label               MasterDarkPath_Label;
                Edit                MasterDarkPath_Edit;
                ToolButton          MasterDarkPath_ToolButton;
                ToolButton          MasterDarkClear_ToolButton;
            HorizontalSizer     MasterFlatPath_Sizer;
                Label               MasterFlatPath_Label;
                Edit                MasterFlatPath_Edit;
                ToolButton          MasterFlatPath_ToolButton;
                ToolButton          MasterFlatClear_ToolButton;
            NumericControl  Pedestal_NumericControl;

        SectionBar      Integration_SectionBar;
        Control         Integration_Control;
        VerticalSizer   Integration_Sizer;
            CheckBox        EnableDigitalAO_CheckBox;
            NumericControl      StarSizeRejectionThreshold_NumericControl;
            NumericControl      StarMovementRejectionThreshold_NumericControl;
            HorizontalSizer     Interpolation_Sizer;
                Label               Interpolation_Lable;
                ComboBox            Interpolation_ComboBox;
            NumericControl      FramePercentage_NumericControl;
            CheckBox            RegistrationOnly_CheckBox;
            HorizontalSizer     RegistrationOutputPath_Sizer;
                Label               RegistrationOutputPath_Label;
                Edit                RegistrationOutputPath_Edit;
                ToolButton          RegistrationOutputPath_ToolButton;
                ToolButton          RegistrationOutputPathClear_ToolButton;
    };

    GUIData* GUI = nullptr;

    void UpdateControls();
    void UpdateRoutineControl();
    void UpdateInputPathControl();
    void UpdateStarControl();
    void UpdateCalibrationControl();
    void UpdateIntegrationControl();
    void UpdateInterpolationControl();

    void __Routine_ItemSelected(ComboBox& /*sender*/, int itemIndex);
    void e_InputPath_Click(Button& sender, bool checked);
    void e_Calibration_Click(Button& sender, bool checked);
    void e_Calibration_EditCompleted(Edit& sender);
    void e_Integration_Click(Button& sender, bool checked);
    void e_RegistrationOutputPath_Click(Button& sender, bool checked);
    void __EditValueUpdated(NumericEdit& sender, double value);
    void __Interpolation_ItemSelected(ComboBox& /*sender*/, int itemIndex);

    friend struct GUIData;
};

PCL_BEGIN_LOCAL
extern LuckyIntegrationInterface* TheLuckyIntegrationInterface;
PCL_END_LOCAL

}	// namespace pcl

#endif  // __LuckyIntegrationInterface_h
