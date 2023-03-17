#include "LuckyIntegrationInterface.h"
#include "LuckyIntegrationParameters.h"
#include "LuckyIntegrationProcess.h"

#include <pcl/ErrorHandler.h>
#include <pcl/FileDialog.h>

namespace pcl
{

LuckyIntegrationInterface* TheLuckyIntegrationInterface = nullptr;

LuckyIntegrationInterface::LuckyIntegrationInterface()
	: m_instance(TheLuckyIntegrationProcess)
{
	TheLuckyIntegrationInterface = this;
}

LuckyIntegrationInterface::~LuckyIntegrationInterface()
{
	if (GUI != nullptr)
		delete GUI, GUI = nullptr;
}

IsoString LuckyIntegrationInterface::Id() const
{
	return "LuckyIntegration";
}

MetaProcess* LuckyIntegrationInterface::Process() const
{
	return TheLuckyIntegrationProcess;
}

IsoString LuckyIntegrationInterface::IconImageSVG() const
{
	return TheLuckyIntegrationProcess->IconImageSVG();
}

InterfaceFeatures LuckyIntegrationInterface::Features() const
{
	return InterfaceFeature::DefaultGlobal;
}

void LuckyIntegrationInterface::ResetInstance()
{
	LuckyIntegrationInstance defaultInstance(TheLuckyIntegrationProcess);
	ImportProcess(defaultInstance);
}

bool LuckyIntegrationInterface::Launch(const MetaProcess& P, const ProcessImplementation*, bool& dynamic, unsigned& /*flags*/)
{
	if (GUI == nullptr)
	{
		GUI = new GUIData(*this);
		SetWindowTitle("LuckyIntegration");
		UpdateControls();

		// Restore position only
		if (!RestoreGeometry())
			SetDefaultPosition();
		AdjustToContents();
	}

	dynamic = false;
	return &P == TheLuckyIntegrationProcess;
}

ProcessImplementation* LuckyIntegrationInterface::NewProcess() const
{
	return new LuckyIntegrationInstance(m_instance);
}

bool LuckyIntegrationInterface::ValidateProcess(const ProcessImplementation& p, String& whyNot) const
{
	if (dynamic_cast<const LuckyIntegrationInstance*>(&p) != nullptr)
		return true;
	whyNot = "Not a LuckyIntegration instance.";
	return false;
}

bool LuckyIntegrationInterface::RequiresInstanceValidation() const
{
	return true;
}

bool LuckyIntegrationInterface::ImportProcess(const ProcessImplementation& p)
{
	m_instance.Assign(p);
	UpdateControls();
	return true;
}

void LuckyIntegrationInterface::SaveSettings() const
{
	SaveGeometry();
}

void LuckyIntegrationInterface::UpdateControls()
{
	UpdateRoutineControl();
	UpdateInputPathControl();
	UpdateStarControl();
	UpdateCalibrationControl();
	UpdateIntegrationControl();
}

void LuckyIntegrationInterface::UpdateRoutineControl()
{
	GUI->Routine_ComboBox.SetCurrentItem(m_instance.p_routine);
	if ((m_instance.p_routine == LIRoutine::StarDetectionPreview) || (m_instance.p_routine == LIRoutine::StarDetectionAlignment))
	{
		GUI->StarDetection_SectionBar.Enable();
		GUI->Calibration_SectionBar.Enable(false);
		GUI->Integration_SectionBar.Enable(false);
	}
	else if (m_instance.p_routine == LIRoutine::ImageIntegration)
	{
		GUI->StarDetection_SectionBar.Enable(false);
		GUI->Calibration_SectionBar.Enable();
		GUI->Integration_SectionBar.Enable();
	}
}

void LuckyIntegrationInterface::UpdateInterpolationControl()
{
	GUI->Interpolation_ComboBox.SetCurrentItem(m_instance.p_interpolation);
}

void LuckyIntegrationInterface::UpdateInputPathControl()
{
	GUI->InputPath_Edit.SetText(m_instance.p_inputPath);
}

void LuckyIntegrationInterface::UpdateStarControl()
{
	GUI->ApproxFWHM_NumericControl.SetValue(m_instance.p_approxFwhm);
	GUI->MinPeak_NumericControl.SetValue(m_instance.p_minPeak);
	GUI->SaturationThreshold_NumericControl.SetValue(m_instance.p_saturationThreshold);
}

void LuckyIntegrationInterface::UpdateCalibrationControl()
{
	GUI->MasterDarkPath_Edit.SetText(m_instance.p_masterDark.path);
	GUI->MasterFlatPath_Edit.SetText(m_instance.p_masterFlat.path);
	GUI->Pedestal_NumericControl.SetValue(m_instance.p_pedestal);
}

void LuckyIntegrationInterface::UpdateIntegrationControl()
{
	GUI->EnableDigitalAO_CheckBox.SetChecked(m_instance.p_enableDigitalAO);
	GUI->StarSizeRejectionThreshold_NumericControl.SetValue(m_instance.p_starSizeRejectionThreshold);
	GUI->StarMovementRejectionThreshold_NumericControl.SetValue(m_instance.p_starMovementRejectionThreshold);
	GUI->FramePercentage_NumericControl.SetValue(m_instance.p_framePercentage);
	GUI->RegistrationOnly_CheckBox.SetChecked(m_instance.p_registrationOnly);
	GUI->RegistrationOutputPath_Edit.SetText(m_instance.p_registrationOutputPath);
}

void LuckyIntegrationInterface::__Routine_ItemSelected(ComboBox& /*sender*/, int itemIndex)
{
	m_instance.p_routine = itemIndex;
	UpdateRoutineControl();
}

void LuckyIntegrationInterface::__Interpolation_ItemSelected(ComboBox& /*sender*/, int itemIndex)
{
	m_instance.p_interpolation = itemIndex;
	UpdateInterpolationControl();
}

void LuckyIntegrationInterface::e_InputPath_Click(Button& sender, bool checked)
{
	if (sender == GUI->InputPath_ToolButton)
	{
		GetDirectoryDialog d;
		d.SetCaption("LuckyIntegration: Select Input Directory");
		if (d.Execute())
		{
			m_instance.p_inputPath = d.Directory();
			UpdateInputPathControl();
		}
	}
	else if (sender == GUI->InputPathClear_ToolButton)
	{
		m_instance.p_inputPath.Clear();
		UpdateInputPathControl();
	}
}

void LuckyIntegrationInterface::e_Calibration_Click(Button& sender, bool checked)
{
	if (sender == GUI->MasterDarkPath_ToolButton)
	{
		OpenFileDialog d;
		d.DisableMultipleSelections();
		d.LoadImageFilters();
		d.SetCaption("LuckyIntegration: Select Master Dark Frame");
		if (d.Execute())
		{
			m_instance.p_masterDark.path = d.FileName();
			UpdateCalibrationControl();
		}
	}
	else if (sender == GUI->MasterDarkClear_ToolButton)
	{
		m_instance.p_masterDark.path.Clear();
		UpdateCalibrationControl();
	}
	else if (sender == GUI->MasterFlatPath_ToolButton)
	{
		OpenFileDialog d;
		d.DisableMultipleSelections();
		d.LoadImageFilters();
		d.SetCaption("LuckyIntegration: Select Master Flat Frame");
		if (d.Execute())
		{
			m_instance.p_masterFlat.path = d.FileName();
			UpdateCalibrationControl();
		}
	}
	else if (sender == GUI->MasterFlatClear_ToolButton)
	{
		m_instance.p_masterFlat.path.Clear();
		UpdateCalibrationControl();
	}
}

void LuckyIntegrationInterface::e_Calibration_EditCompleted(Edit& sender)
{
	String text = sender.Text().Trimmed();

	if (sender == GUI->MasterDarkPath_Edit)
		m_instance.p_masterDark.path = text;
	else if (sender == GUI->MasterFlatPath_Edit)
		m_instance.p_masterFlat.path = text;

	sender.SetText(text);
}

void LuckyIntegrationInterface::e_Integration_Click(Button& sender, bool checked)
{
	if (sender == GUI->EnableDigitalAO_CheckBox)
		m_instance.p_enableDigitalAO = checked;
	else if (sender == GUI->RegistrationOnly_CheckBox)
		m_instance.p_registrationOnly = checked;
	UpdateIntegrationControl();
}

void LuckyIntegrationInterface::e_RegistrationOutputPath_Click(Button& sender, bool checked)
{
	if (sender == GUI->RegistrationOutputPath_ToolButton)
	{
		GetDirectoryDialog d;
		d.SetCaption("LuckyIntegration: Select Registration Output Directory");
		if (d.Execute())
		{
			m_instance.p_registrationOutputPath = d.Directory();
			UpdateIntegrationControl();
		}
	}
	else if (sender == GUI->RegistrationOutputPathClear_ToolButton)
	{
		m_instance.p_registrationOutputPath.Clear();
		UpdateIntegrationControl();
	}
}

void LuckyIntegrationInterface::__EditValueUpdated(NumericEdit& sender, double value)
{
	if (sender == GUI->ApproxFWHM_NumericControl)
		m_instance.p_approxFwhm = value;
	else if (sender == GUI->MinPeak_NumericControl)
		m_instance.p_minPeak = value;
	else if (sender == GUI->SaturationThreshold_NumericControl)
		m_instance.p_saturationThreshold = value;
	else if (sender == GUI->Pedestal_NumericControl)
		m_instance.p_pedestal = value;
	else if (sender == GUI->StarSizeRejectionThreshold_NumericControl)
		m_instance.p_starSizeRejectionThreshold = value;
	else if (sender == GUI->StarMovementRejectionThreshold_NumericControl)
		m_instance.p_starMovementRejectionThreshold = value;
	else if (sender == GUI->FramePercentage_NumericControl)
		m_instance.p_framePercentage = value;
}

LuckyIntegrationInterface::GUIData::GUIData(LuckyIntegrationInterface& w)
{
	pcl::Font fnt = w.Font();
	int labelWidth1 = fnt.Width(String("Star Movement Rejection Threshold:") + 'M');
	int editWidth1 = fnt.Width(String('M', 16));
	int editWidth2 = fnt.Width(String('M', 45));

	// Routine
	const char* routineToolTip = "<p><b>Star Detection Preview</b>: Detect stars on the first input image and show the detection for preview purpose.</p>"
								 "<p><b>Star Detection & Alignment</b>: Detect and align stars in each input image and save the results to an XML file.</p>"
								 "<p><b>Image Integration</b>: Correct the movement of stars and apply average combination on the input images, using the results of star detection and alignment.</p>";
	Routine_Lable.SetText("Routine:");
	Routine_Lable.SetFixedWidth(labelWidth1);
	Routine_Lable.SetTextAlignment(TextAlign::Right | TextAlign::VertCenter);
	Routine_Lable.SetToolTip(routineToolTip);
	Routine_ComboBox.AddItem("Star Detection Preview");
	Routine_ComboBox.AddItem("Star Detection & Alignment");
	Routine_ComboBox.AddItem("Image Integration");
	Routine_ComboBox.SetToolTip(routineToolTip);
	Routine_ComboBox.OnItemSelected((ComboBox::item_event_handler) & LuckyIntegrationInterface::__Routine_ItemSelected, w);
	Routine_Sizer.SetSpacing(4);
	Routine_Sizer.Add(Routine_Lable);
	Routine_Sizer.Add(Routine_ComboBox);
	Routine_Sizer.AddStretch();

	// Section "Input Images"
	InputPath_SectionBar.SetTitle("Input");
	InputPath_SectionBar.SetSection(InputPath_Control);

	const char* InputPathToolTip = "<p>Directory of the input images. Only .fit and .fits images will be processed.</p>";

	InputPath_Label.SetText("Input Directory:");
	InputPath_Label.SetFixedWidth(labelWidth1);
	InputPath_Label.SetTextAlignment(TextAlign::Right | TextAlign::VertCenter);
	InputPath_Label.SetToolTip(InputPathToolTip);

	InputPath_Edit.SetToolTip(InputPathToolTip);
	InputPath_Edit.SetMinWidth(editWidth2);
	InputPath_Edit.SetReadOnly();

	InputPath_ToolButton.SetIcon(w.ScaledResource(":/icons/select-file.png"));
	InputPath_ToolButton.SetScaledFixedSize(20, 20);
	InputPath_ToolButton.SetToolTip("<p>Select input directory.</p>");
	InputPath_ToolButton.OnClick((Button::click_event_handler) & LuckyIntegrationInterface::e_InputPath_Click, w);

	InputPathClear_ToolButton.SetIcon(w.ScaledResource(":/icons/window-close.png"));
	InputPathClear_ToolButton.SetScaledFixedSize(20, 20);
	InputPathClear_ToolButton.SetToolTip("<p>Clear input directory.</p>");
	InputPathClear_ToolButton.OnClick((Button::click_event_handler) & LuckyIntegrationInterface::e_InputPath_Click, w);

	InputPath_Sizer.SetSpacing(4);
	InputPath_Sizer.Add(InputPath_Label);
	InputPath_Sizer.Add(InputPath_Edit, 100);
	InputPath_Sizer.Add(InputPath_ToolButton);
	InputPath_Sizer.Add(InputPathClear_ToolButton);
	InputPath_Sizer.AddStretch();

	InputPath_Control.SetSizer(InputPath_Sizer);

	// Section "Star Detection"
	StarDetection_SectionBar.SetTitle("Star Detection");
	StarDetection_SectionBar.SetSection(StarDetection_Control);

	ApproxFWHM_NumericControl.label.SetText("Approximate FWHM:");
	ApproxFWHM_NumericControl.label.SetFixedWidth(labelWidth1);
	ApproxFWHM_NumericControl.slider.SetRange(0, 500);
	ApproxFWHM_NumericControl.slider.SetScaledMinWidth(300);
	ApproxFWHM_NumericControl.SetReal();
	ApproxFWHM_NumericControl.SetRange(TheLIApproxFWHMParameter->MinimumValue(), TheLIApproxFWHMParameter->MaximumValue());
	ApproxFWHM_NumericControl.SetPrecision(TheLIApproxFWHMParameter->Precision());
	ApproxFWHM_NumericControl.edit.SetFixedWidth(editWidth1);
	ApproxFWHM_NumericControl.SetToolTip("<p>Approximate FWHM (in pixels) of stars.</p>");
	ApproxFWHM_NumericControl.OnValueUpdated((NumericEdit::value_event_handler)&LuckyIntegrationInterface::__EditValueUpdated, w);

	MinPeak_NumericControl.label.SetText("Minimum Peak:");
	MinPeak_NumericControl.label.SetFixedWidth(labelWidth1);
	MinPeak_NumericControl.slider.SetRange(0, 500);
	MinPeak_NumericControl.slider.SetScaledMinWidth(300);
	MinPeak_NumericControl.SetReal();
	MinPeak_NumericControl.SetRange(TheLIMinPeakParameter->MinimumValue(), TheLIMinPeakParameter->MaximumValue());
	MinPeak_NumericControl.SetPrecision(TheLIMinPeakParameter->Precision());
	MinPeak_NumericControl.edit.SetFixedWidth(editWidth1);
	MinPeak_NumericControl.SetToolTip("<p>A star with peak value below this threshold will be excluded.</p>");
	MinPeak_NumericControl.OnValueUpdated((NumericEdit::value_event_handler)&LuckyIntegrationInterface::__EditValueUpdated, w);

	SaturationThreshold_NumericControl.label.SetText("Saturation Threshold:");
	SaturationThreshold_NumericControl.label.SetFixedWidth(labelWidth1);
	SaturationThreshold_NumericControl.slider.SetRange(0, 500);
	SaturationThreshold_NumericControl.slider.SetScaledMinWidth(300);
	SaturationThreshold_NumericControl.SetReal();
	SaturationThreshold_NumericControl.SetRange(TheLISaturationThresholdParameter->MinimumValue(), TheLISaturationThresholdParameter->MaximumValue());
	SaturationThreshold_NumericControl.SetPrecision(TheLISaturationThresholdParameter->Precision());
	SaturationThreshold_NumericControl.edit.SetFixedWidth(editWidth1);
	SaturationThreshold_NumericControl.SetToolTip("<p>A star with peak value above this threshold will be excluded.</p>");
	SaturationThreshold_NumericControl.OnValueUpdated((NumericEdit::value_event_handler)&LuckyIntegrationInterface::__EditValueUpdated, w);

	StarDetection_Sizer.SetSpacing(4);
	StarDetection_Sizer.Add(ApproxFWHM_NumericControl);
	StarDetection_Sizer.Add(MinPeak_NumericControl);
	StarDetection_Sizer.Add(SaturationThreshold_NumericControl);
	StarDetection_Sizer.AddStretch();

	StarDetection_Control.SetSizer(StarDetection_Sizer);

	// Section "Calibration"
	Calibration_SectionBar.SetTitle("Calibration");
	Calibration_SectionBar.SetSection(Calibration_Control);

	const char* masterDarkToolTip = "<p>File path of the master dark frame.</p>";

	MasterDarkPath_Label.SetText("Master Dark: ");
	MasterDarkPath_Label.SetFixedWidth(labelWidth1);
	MasterDarkPath_Label.SetTextAlignment(TextAlign::Right | TextAlign::VertCenter);
	MasterDarkPath_Label.SetToolTip(masterDarkToolTip);

	MasterDarkPath_Edit.SetToolTip(masterDarkToolTip);
	MasterDarkPath_Edit.SetMinWidth(editWidth2);
	MasterDarkPath_Edit.SetReadOnly();

	MasterDarkPath_ToolButton.SetIcon(w.ScaledResource(":/icons/select-file.png"));
	MasterDarkPath_ToolButton.SetScaledFixedSize(20, 20);
	MasterDarkPath_ToolButton.SetToolTip("<p>Select the master dark image file.</p>");
	MasterDarkPath_ToolButton.OnClick((Button::click_event_handler) & LuckyIntegrationInterface::e_Calibration_Click, w);

	MasterDarkClear_ToolButton.SetIcon(w.ScaledResource(":/icons/window-close.png"));
	MasterDarkClear_ToolButton.SetScaledFixedSize(20, 20);
	MasterDarkClear_ToolButton.SetToolTip("<p>Deselect master dark</p>");
	MasterDarkClear_ToolButton.OnClick((Button::click_event_handler) & LuckyIntegrationInterface::e_Calibration_Click, w);

	MasterDarkPath_Sizer.SetSpacing(4);
	MasterDarkPath_Sizer.Add(MasterDarkPath_Label);
	MasterDarkPath_Sizer.Add(MasterDarkPath_Edit, 100);
	MasterDarkPath_Sizer.Add(MasterDarkPath_ToolButton);
	MasterDarkPath_Sizer.Add(MasterDarkClear_ToolButton);
	MasterDarkPath_Sizer.AddStretch();

	const char* masterFlatToolTip = "<p>File path of the master flat frame.</p>";

	MasterFlatPath_Label.SetText("Master Flat: ");
	MasterFlatPath_Label.SetFixedWidth(labelWidth1);
	MasterFlatPath_Label.SetTextAlignment(TextAlign::Right | TextAlign::VertCenter);
	MasterFlatPath_Label.SetToolTip(masterFlatToolTip);

	MasterFlatPath_Edit.SetToolTip(masterFlatToolTip);
	MasterFlatPath_Edit.SetMinWidth(editWidth2);
	MasterFlatPath_Edit.SetReadOnly();

	MasterFlatPath_ToolButton.SetIcon(w.ScaledResource(":/icons/select-file.png"));
	MasterFlatPath_ToolButton.SetScaledFixedSize(20, 20);
	MasterFlatPath_ToolButton.SetToolTip("<p>Select the master flat image file.</p>");
	MasterFlatPath_ToolButton.OnClick((Button::click_event_handler) & LuckyIntegrationInterface::e_Calibration_Click, w);

	MasterFlatClear_ToolButton.SetIcon(w.ScaledResource(":/icons/window-close.png"));
	MasterFlatClear_ToolButton.SetScaledFixedSize(20, 20);
	MasterFlatClear_ToolButton.SetToolTip("<p>Deselect master flat</p>");
	MasterFlatClear_ToolButton.OnClick((Button::click_event_handler)& LuckyIntegrationInterface::e_Calibration_Click, w);

	MasterFlatPath_Sizer.SetSpacing(4);
	MasterFlatPath_Sizer.Add(MasterFlatPath_Label);
	MasterFlatPath_Sizer.Add(MasterFlatPath_Edit, 100);
	MasterFlatPath_Sizer.Add(MasterFlatPath_ToolButton);
	MasterFlatPath_Sizer.Add(MasterFlatClear_ToolButton);
	MasterFlatPath_Sizer.AddStretch();

	Pedestal_NumericControl.label.SetText("Calibration Pedestal:");
	Pedestal_NumericControl.label.SetFixedWidth(labelWidth1);
	Pedestal_NumericControl.slider.SetRange(0, 500);
	Pedestal_NumericControl.slider.SetScaledMinWidth(300);
	Pedestal_NumericControl.SetReal();
	Pedestal_NumericControl.SetRange(TheLIPedestalParameter->MinimumValue(), TheLIPedestalParameter->MaximumValue());
	Pedestal_NumericControl.SetPrecision(TheLIPedestalParameter->Precision());
	Pedestal_NumericControl.edit.SetFixedWidth(editWidth1);
	Pedestal_NumericControl.SetToolTip("<p>Pedestal to be added to the calibrated images. Ignored if not having master dark.</p>");
	Pedestal_NumericControl.OnValueUpdated((NumericEdit::value_event_handler)&LuckyIntegrationInterface::__EditValueUpdated, w);

	Calibration_Sizer.SetSpacing(4);
	Calibration_Sizer.Add(MasterDarkPath_Sizer);
	Calibration_Sizer.Add(MasterFlatPath_Sizer);
	Calibration_Sizer.Add(Pedestal_NumericControl);

	Calibration_Control.SetSizer(Calibration_Sizer);

	// Section "Integration"
	Integration_SectionBar.SetTitle("Integration");
	Integration_SectionBar.SetSection(Integration_Control);

	EnableDigitalAO_CheckBox.SetText("Enable Digital AO");
	EnableDigitalAO_CheckBox.SetToolTip("<p>Enable digital AO (adaptive optics).</p>"
										"<p>When enabled, registration will be done by correcting the movement of each detected star. Pixel rejection will be done independently for each star.</p>"
										"<p>When disabled, registration and pixel rejection will be done based on the average movement and size of all detected stars.</p>");
	EnableDigitalAO_CheckBox.OnClick((Button::click_event_handler)& LuckyIntegrationInterface::e_Integration_Click, w);

	StarSizeRejectionThreshold_NumericControl.label.SetText("Star Size Rejection Threshold:");
	StarSizeRejectionThreshold_NumericControl.label.SetFixedWidth(labelWidth1);
	StarSizeRejectionThreshold_NumericControl.slider.SetRange(0, 500);
	StarSizeRejectionThreshold_NumericControl.slider.SetScaledMinWidth(300);
	StarSizeRejectionThreshold_NumericControl.SetReal();
	StarSizeRejectionThreshold_NumericControl.SetRange(TheLIStarSizeRejectionThresholdParameter->MinimumValue(), TheLIStarSizeRejectionThresholdParameter->MaximumValue());
	StarSizeRejectionThreshold_NumericControl.SetPrecision(TheLIStarSizeRejectionThresholdParameter->Precision());
	StarSizeRejectionThreshold_NumericControl.edit.SetFixedWidth(editWidth1);
	StarSizeRejectionThreshold_NumericControl.SetToolTip("<p>Any star with size (in pixels) above this threshold is considered disqualified. Pixels near any disqualified stars will be rejected during integration.</p>");
	StarSizeRejectionThreshold_NumericControl.OnValueUpdated((NumericEdit::value_event_handler)& LuckyIntegrationInterface::__EditValueUpdated, w);

	StarMovementRejectionThreshold_NumericControl.label.SetText("Star Movement Rejection Threshold:");
	StarMovementRejectionThreshold_NumericControl.label.SetFixedWidth(labelWidth1);
	StarMovementRejectionThreshold_NumericControl.slider.SetRange(0, 500);
	StarMovementRejectionThreshold_NumericControl.slider.SetScaledMinWidth(300);
	StarMovementRejectionThreshold_NumericControl.SetReal();
	StarMovementRejectionThreshold_NumericControl.SetRange(TheLIStarMovementRejectionThresholdParameter->MinimumValue(), TheLIStarMovementRejectionThresholdParameter->MaximumValue());
	StarMovementRejectionThreshold_NumericControl.SetPrecision(TheLIStarMovementRejectionThresholdParameter->Precision());
	StarMovementRejectionThreshold_NumericControl.edit.SetFixedWidth(editWidth1);
	StarMovementRejectionThreshold_NumericControl.SetToolTip("<p>Any star with movement (in pixels, comparing to previous frame) above this threshold is considered disqualified. Pixels near any disqualified stars will be rejected during integration.</p>");
	StarMovementRejectionThreshold_NumericControl.OnValueUpdated((NumericEdit::value_event_handler)& LuckyIntegrationInterface::__EditValueUpdated, w);

	// Routine
	const char* interpolationToolTip = "<p><b>Nearest</b>: No interpolation. Fastest processing but poor results.</p>"
									   "<p><b>Bilinear</b>: Suitable for low SNR images. Fast with OK results.</p>"
									   "<p><b>Lanczos</b>: Suitable for medium to high SNR images. Best interpolation algorithm for image registration without rescaling. Slow processing but best results.</p>";
	Interpolation_Lable.SetText("Interpolation:");
	Interpolation_Lable.SetFixedWidth(labelWidth1);
	Interpolation_Lable.SetTextAlignment(TextAlign::Right | TextAlign::VertCenter);
	Interpolation_Lable.SetToolTip(interpolationToolTip);
	Interpolation_ComboBox.AddItem("Nearest");
	Interpolation_ComboBox.AddItem("Bilinear");
	Interpolation_ComboBox.AddItem("Lanczos-3");
	Interpolation_ComboBox.SetToolTip(interpolationToolTip);
	Interpolation_ComboBox.OnItemSelected((ComboBox::item_event_handler)&LuckyIntegrationInterface::__Interpolation_ItemSelected, w);
	Interpolation_Sizer.SetSpacing(4);
	Interpolation_Sizer.Add(Interpolation_Lable);
	Interpolation_Sizer.Add(Interpolation_ComboBox);
	Interpolation_Sizer.AddStretch();

	FramePercentage_NumericControl.label.SetText("Frame Percentage to Process:");
	FramePercentage_NumericControl.label.SetFixedWidth(labelWidth1);
	FramePercentage_NumericControl.slider.SetRange(0, 500);
	FramePercentage_NumericControl.slider.SetScaledMinWidth(300);
	FramePercentage_NumericControl.SetReal();
	FramePercentage_NumericControl.SetRange(TheLIFramePercentageParameter->MinimumValue(), TheLIFramePercentageParameter->MaximumValue());
	FramePercentage_NumericControl.SetPrecision(TheLIFramePercentageParameter->Precision());
	FramePercentage_NumericControl.edit.SetFixedWidth(editWidth1);
	FramePercentage_NumericControl.SetToolTip("<p>Specify how many frames in the input directory will be processed, in percentage.</p>");
	FramePercentage_NumericControl.OnValueUpdated((NumericEdit::value_event_handler)&LuckyIntegrationInterface::__EditValueUpdated, w);

	RegistrationOnly_CheckBox.SetText("Registration Only");
	RegistrationOnly_CheckBox.SetToolTip("<p>Perform registration only.</p>"
		"<p>When enabled, integration will be skipped, and registration result of each frame will be saved to the output directory which is specified below.</p>");
	RegistrationOnly_CheckBox.OnClick((Button::click_event_handler)&LuckyIntegrationInterface::e_Integration_Click, w);

	const char* RegistrationOutputPathToolTip = "<p>Directory of the registration output images.</p>";

	RegistrationOutputPath_Label.SetText("Registration Output Directory:");
	RegistrationOutputPath_Label.SetFixedWidth(labelWidth1);
	RegistrationOutputPath_Label.SetTextAlignment(TextAlign::Right | TextAlign::VertCenter);
	RegistrationOutputPath_Label.SetToolTip(RegistrationOutputPathToolTip);

	RegistrationOutputPath_Edit.SetToolTip(RegistrationOutputPathToolTip);
	RegistrationOutputPath_Edit.SetMinWidth(editWidth2);
	RegistrationOutputPath_Edit.SetReadOnly();

	RegistrationOutputPath_ToolButton.SetIcon(w.ScaledResource(":/icons/select-file.png"));
	RegistrationOutputPath_ToolButton.SetScaledFixedSize(20, 20);
	RegistrationOutputPath_ToolButton.SetToolTip("<p>Select input directory.</p>");
	RegistrationOutputPath_ToolButton.OnClick((Button::click_event_handler)&LuckyIntegrationInterface::e_RegistrationOutputPath_Click, w);

	RegistrationOutputPathClear_ToolButton.SetIcon(w.ScaledResource(":/icons/window-close.png"));
	RegistrationOutputPathClear_ToolButton.SetScaledFixedSize(20, 20);
	RegistrationOutputPathClear_ToolButton.SetToolTip("<p>Clear input directory.</p>");
	RegistrationOutputPathClear_ToolButton.OnClick((Button::click_event_handler)&LuckyIntegrationInterface::e_RegistrationOutputPath_Click, w);

	RegistrationOutputPath_Sizer.SetSpacing(4);
	RegistrationOutputPath_Sizer.Add(RegistrationOutputPath_Label);
	RegistrationOutputPath_Sizer.Add(RegistrationOutputPath_Edit, 100);
	RegistrationOutputPath_Sizer.Add(RegistrationOutputPath_ToolButton);
	RegistrationOutputPath_Sizer.Add(RegistrationOutputPathClear_ToolButton);
	RegistrationOutputPath_Sizer.AddStretch();

	Integration_Sizer.SetSpacing(4);
	Integration_Sizer.Add(EnableDigitalAO_CheckBox);
	Integration_Sizer.Add(StarSizeRejectionThreshold_NumericControl);
	Integration_Sizer.Add(StarMovementRejectionThreshold_NumericControl);
	Integration_Sizer.Add(Interpolation_Sizer);
	Integration_Sizer.Add(FramePercentage_NumericControl);
	Integration_Sizer.Add(RegistrationOnly_CheckBox);
	Integration_Sizer.Add(RegistrationOutputPath_Sizer);
	Integration_Sizer.AddStretch();

	Integration_Control.SetSizer(Integration_Sizer);

	Global_Sizer.SetMargin(8);
	Global_Sizer.SetSpacing(6);
	Global_Sizer.Add(Routine_Sizer);
	Global_Sizer.Add(InputPath_SectionBar);
	Global_Sizer.Add(InputPath_Control);
	Global_Sizer.Add(StarDetection_SectionBar);
	Global_Sizer.Add(StarDetection_Control);
	Global_Sizer.Add(Calibration_SectionBar);
	Global_Sizer.Add(Calibration_Control);
	Global_Sizer.Add(Integration_SectionBar);
	Global_Sizer.Add(Integration_Control);

	w.SetSizer(Global_Sizer);

	w.EnsureLayoutUpdated();
	w.AdjustToContents();
	w.SetFixedSize();
}

}	// namespace pcl