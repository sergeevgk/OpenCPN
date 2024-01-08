/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Canvas Options Window/Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "wx/artprov.h"
#include <wx/statline.h>


#include "dychart.h"

#include "chcanv.h"
#include "CanvasOptions.h"
#include "OCPNPlatform.h"

#ifdef ocpnUSE_SVG
#include "wxSVG/svg.h"
#endif // ocpnUSE_SVG

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif



//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------


//  Helper utilities


//  Helper classes


//------------------------------------------------------------------------------
//    CanvasOptions
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CanvasOptions, wxDialog)
EVT_CLOSE(CanvasOptions::OnClose)
//EVT_CHECKBOX(ID_QUILTCHECKBOX1, CanvasOptions::OnOptionChange)
END_EVENT_TABLE()

CanvasOptions::CanvasOptions( wxWindow *parent)
 :wxDialog()

{
    m_ENCAvail = true;
    
    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );

    //SetBackgroundStyle( wxBG_STYLE_TRANSPARENT );
    long mstyle = wxNO_BORDER | wxFRAME_NO_TASKBAR | wxFRAME_SHAPED;
    wxDialog::Create(parent, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, mstyle);
    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
    SetSizer(topsizer);
    
    m_sWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxSUNKEN_BORDER);
    topsizer->Add(m_sWindow, 1, wxEXPAND );
    
    m_sWindow->SetScrollRate(0, 5);
    
    int border_size = 4;
    int group_item_spacing = 0;
    int interGroupSpace = border_size * 2;
    int interBoxSpace = 5;
    
    wxSizerFlags verticalInputFlags = wxSizerFlags(0).Align(wxALIGN_LEFT).Expand().Border(wxALL, group_item_spacing);
    wxSizerFlags inputFlags = wxSizerFlags(0).Align(wxALIGN_LEFT).Border(wxALL, group_item_spacing);
    
    wxScrolledWindow *pDisplayPanel = m_sWindow;

    wxBoxSizer* generalSizer = new wxBoxSizer(wxVERTICAL);
    pDisplayPanel->SetSizer(generalSizer);

    
    //  Options Label
    wxStaticText* optionsLabelBox = new wxStaticText(pDisplayPanel, wxID_ANY, _("Chart Panel Options"));
    generalSizer->Add(optionsLabelBox, 0, wxALL | wxEXPAND , border_size);
    wxStaticLine *m_staticLine121 = new wxStaticLine(pDisplayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    generalSizer->Add(m_staticLine121, 0, wxALL | wxEXPAND , border_size);
    
    // spacer
    generalSizer->Add(0, interGroupSpace);

    // Control Options
//    wxStaticBoxSizer* boxCont = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("Control Options")), wxVERTICAL);
//    generalSizer->Add(boxCont, 0, wxALL | wxEXPAND, border_size);
    
//     pCBToolbar = new wxCheckBox(pDisplayPanel, ID_TOOLBARCHECKBOX, _("Show Toolbar"));
//     boxCont->Add(pCBToolbar, verticalInputFlags);
//     pCBToolbar->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
// 
//     // spacer
//     generalSizer->Add(0, interGroupSpace);

	// Weather Mode
	wxStaticBoxSizer* boxWeatherMode = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("Weather Mode")), wxVERTICAL);
	generalSizer->Add(boxWeatherMode, 0, wxALL | wxEXPAND, border_size);
	//wxRadioButton *pWaveHeight, *pRippleHeight, *WaveRippleHeight, *pWind;
	wxBoxSizer* rowOrientationWeather = new wxBoxSizer(wxVERTICAL);
	boxWeatherMode->Add(rowOrientationWeather);

	rowOrientationWeather->AddSpacer(interBoxSpace);

	pCBDrawWaveHeight = new wxCheckBox(pDisplayPanel, IDCO_DRAW_WAVE_HEIGHT, _("Wave height gradient"));
	rowOrientationWeather->Add(pCBDrawWaveHeight, verticalInputFlags);
	pCBDrawWaveHeight->SetValue(false);
	pCBDrawWaveHeight->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);
	
	rowOrientationWeather->AddSpacer(interBoxSpace);


	pWaveHeight = new wxRadioButton(pDisplayPanel, wxID_ANY, _("Wave Height"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	rowOrientationWeather->Add(pWaveHeight, inputFlags);
	pWaveHeight->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);

	pRippleHeight = new wxRadioButton(pDisplayPanel, IDCO_RIPPLEHEIGHTCHECKBOX, _("Ripple Height"));
	rowOrientationWeather->Add(pRippleHeight, wxSizerFlags(0).Align(wxALIGN_LEFT).Border(wxLEFT, group_item_spacing * 2));
	pRippleHeight->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);

	pWaveRippleHeight = new wxRadioButton(pDisplayPanel, IDCO_WAVERIPPLEHEIGHTCHECKBOX, _("Wave + Ripple Height"));
	rowOrientationWeather->Add(pWaveRippleHeight, wxSizerFlags(0).Align(wxALIGN_LEFT).Border(wxLEFT, group_item_spacing * 2));
	pWaveRippleHeight->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);
	
	rowOrientationWeather->AddSpacer(interBoxSpace);


#if !defined(__WXOSX__)  
#define SLIDER_STYLE  wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS
#else
#define SLIDER_STYLE  wxSL_HORIZONTAL | wxSL_AUTOTICKS
#endif
	wxSize m_sliderSize = wxSize(20 * 8, 10 * 5);

	rowOrientationWeather->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("Danger Height, cm")), inputFlags);

	pSliderDangerHeight = new wxSlider(pDisplayPanel, ID_DANGERHEIGHT, 0, 0, 600,
		wxDefaultPosition, m_sliderSize, SLIDER_STYLE);

	rowOrientationWeather->Add(pSliderDangerHeight, inputFlags);
	pSliderDangerHeight->Connect(wxEVT_SLIDER, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);

	rowOrientationWeather->AddSpacer(interBoxSpace);

	//pChoiceDateTime
	rowOrientationWeather->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("Date \& Time")), inputFlags);

	pChoiceDateTime = new wxChoice();
	wxArrayString labels;
	wxString label("no data");
	labels.Add(label);
	wxSize m_choiceSize = wxSize(20 * 7, 10 * 2);

	if (pChoiceDateTime) {
		pChoiceDateTime->Create(pDisplayPanel,
			wxID_ANY,
			wxDefaultPosition,
			m_choiceSize,
			labels,
			wxCB_SORT);
		int index_selected = 0;
		pChoiceDateTime->SetSelection(index_selected);
		rowOrientationWeather->Add(pChoiceDateTime, inputFlags);
		pChoiceDateTime->Show();
	}
	pChoiceDateTime->Connect(wxEVT_CHOICE, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);
	
	rowOrientationWeather->AddSpacer(interBoxSpace);

	pCBCheckRoute = new wxCheckBox(pDisplayPanel, IDCO_CHECKROUTE, _("Check Route"));
	rowOrientationWeather->Add(pCBCheckRoute, verticalInputFlags);
	pCBCheckRoute->SetValue(false);
	pCBCheckRoute->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);

	pCBCalculateRoute = new wxCheckBox(pDisplayPanel, IDCO_CALCULATEROUTE, _("Calculate Route"));
	rowOrientationWeather->Add(pCBCalculateRoute, verticalInputFlags);
	pCBCalculateRoute->SetValue(false);
	pCBCalculateRoute->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);
	
	pCBCalculateFuelRoute = new wxCheckBox(pDisplayPanel, IDCO_CALCULATEFUELROUTE, _("Calculate Fuel Rate Route"));
	rowOrientationWeather->Add(pCBCalculateFuelRoute, verticalInputFlags);
	pCBCalculateFuelRoute->SetValue(false);
	pCBCalculateFuelRoute->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);

	pCBCheckOptimalRoute = new wxCheckBox(pDisplayPanel, IDCO_CHECKOPTIMALROUTE, _("Check Optimal Route"));
	rowOrientationWeather->Add(pCBCheckOptimalRoute, verticalInputFlags);
	pCBCheckOptimalRoute->SetValue(false);
	pCBCheckOptimalRoute->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);

	rowOrientationWeather->AddSpacer(interBoxSpace);

	rowOrientationWeather->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("Start Time")), inputFlags);

	pChoiceStartTime = new wxChoice();
	wxArrayString labelsStart;
	wxString labelStart("no data");
	labelsStart.Add(labelStart);
	wxSize m_choiceSizeStart = wxSize(20 * 7, 10 * 2);

	if (pChoiceStartTime) {
		pChoiceStartTime->Create(pDisplayPanel,
			wxID_ANY,
			wxDefaultPosition,
			m_choiceSizeStart,
			labelsStart,
			wxCB_SORT);
		int index_selected = 0;
		pChoiceStartTime->SetSelection(index_selected);
		rowOrientationWeather->Add(pChoiceStartTime, inputFlags);
		pChoiceStartTime->Show();
	}
	pChoiceStartTime->Connect(wxEVT_CHOICE, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);

	rowOrientationWeather->AddSpacer(interBoxSpace);

	time_t tempi(10);
	wxDateTime zero(tempi);
	zero.ResetTime();
	pThreeHoursTime = new wxTimePickerCtrl(pDisplayPanel, ID_THREEHOURSTIME, zero.GetDateOnly(), wxDefaultPosition, wxDefaultSize, 0);
	rowOrientationWeather->Add(pThreeHoursTime, verticalInputFlags);
	pThreeHoursTime->Connect(wxEVT_TIME_CHANGED, wxCommandEventHandler(CanvasOptions::OnOptionChange), NULL, this);

	// spacer
	generalSizer->Add(0, interGroupSpace);

	wxStaticBoxSizer* boxShipInfo = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("Ship Info")), wxVERTICAL);
	generalSizer->Add(boxShipInfo, 0, wxALL | wxEXPAND, border_size);

	wxBoxSizer* rowOrientationShipInfo = new wxBoxSizer(wxVERTICAL);
	boxShipInfo->Add(rowOrientationShipInfo);

	rowOrientationShipInfo->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("Danger Height, cm")), inputFlags);

	pShipDangerHeight = new wxSpinCtrl(pDisplayPanel, ID_SHIPDANGERHEIGHT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 600, 0);
	rowOrientationShipInfo->Add(pShipDangerHeight, verticalInputFlags);

	rowOrientationShipInfo->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("N, kw")), inputFlags);

	pShipN = new wxSpinCtrl(pDisplayPanel, ID_SHIPN, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 20000, 0);
	rowOrientationShipInfo->Add(pShipN, verticalInputFlags);

	rowOrientationShipInfo->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("D, ton")), inputFlags);

	pShipD = new wxSpinCtrl(pDisplayPanel, ID_SHIPD, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 30000, 0);
	rowOrientationShipInfo->Add(pShipD, verticalInputFlags);

	rowOrientationShipInfo->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("L, m")), inputFlags);

	pShipL = new wxSpinCtrl(pDisplayPanel, ID_SHIPL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 200, 0);
	rowOrientationShipInfo->Add(pShipL, verticalInputFlags);

	rowOrientationShipInfo->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("delta, 10-3")), inputFlags);

	pShipDelta = new wxSpinCtrl(pDisplayPanel, ID_SHIPDELTA, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1000, 0);
	rowOrientationShipInfo->Add(pShipDelta, verticalInputFlags);

	rowOrientationShipInfo->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("speed, knot")), inputFlags);
	pShipSpeed = new wxSpinCtrl(pDisplayPanel, ID_SHIPSPEED, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 200, 0);
	rowOrientationShipInfo->Add(pShipSpeed, verticalInputFlags);

	rowOrientationShipInfo->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("draft, ft")), inputFlags);
	pShipDraft = new wxSpinCtrl(pDisplayPanel, ID_SHIPDRAFT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
	rowOrientationShipInfo->Add(pShipDraft, verticalInputFlags);



	// spacer
	generalSizer->Add(0, interGroupSpace);
    
    // Nav Mode
    wxStaticBoxSizer* boxNavMode = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("Navigation Mode")), wxVERTICAL);
    generalSizer->Add(boxNavMode, 0, wxALL | wxEXPAND, border_size);
    
    wxBoxSizer* rowOrientation = new wxBoxSizer(wxVERTICAL);
    boxNavMode->Add(rowOrientation);

    pCBNorthUp = new wxRadioButton(pDisplayPanel, wxID_ANY, _("North Up"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    rowOrientation->Add(pCBNorthUp, inputFlags);
    pCBNorthUp->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );

    pCBCourseUp = new wxRadioButton(pDisplayPanel, IDCO_COURSEUPCHECKBOX, _("Course Up"));
    rowOrientation->Add(pCBCourseUp, wxSizerFlags(0).Align(wxALIGN_LEFT).Border(wxLEFT, group_item_spacing * 2));
    pCBCourseUp->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
 
    pCBHeadUp = new wxRadioButton(pDisplayPanel, IDCO_HEADUPCHECKBOX, _("Heading Up"));
    rowOrientation->Add(pCBHeadUp, wxSizerFlags(0).Align(wxALIGN_LEFT).Border(wxLEFT, group_item_spacing * 2));
    pCBHeadUp->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
 
    pCBLookAhead = new wxCheckBox(pDisplayPanel, IDCO_CHECK_LOOKAHEAD, _("Look Ahead Mode"));
    boxNavMode->Add(pCBLookAhead, verticalInputFlags);
    pCBLookAhead->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    // spacer
    generalSizer->Add(0, interGroupSpace);

    
    // Display Options
    wxStaticBoxSizer* boxDisp = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("Display Options")), wxVERTICAL);
    generalSizer->Add(boxDisp, 0, wxALL | wxEXPAND, border_size);
    
    pCDOQuilting = new wxCheckBox(pDisplayPanel, IDCO_QUILTCHECKBOX1, _("Enable Chart Quilting"));
    boxDisp->Add(pCDOQuilting, verticalInputFlags);
    pCDOQuilting->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pSDisplayGrid = new wxCheckBox(pDisplayPanel, IDCO_CHECK_DISPLAYGRID, _("Show Grid"));
    boxDisp->Add(pSDisplayGrid, verticalInputFlags);
    pSDisplayGrid->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pCDOOutlines = new wxCheckBox(pDisplayPanel, IDCO_OUTLINECHECKBOX1, _("Show Chart Outlines"));
    boxDisp->Add(pCDOOutlines, verticalInputFlags);
    pCDOOutlines->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pSDepthUnits = new wxCheckBox(pDisplayPanel, IDCO_SHOWDEPTHUNITSBOX1, _("Show Depth Units"));
    boxDisp->Add(pSDepthUnits, verticalInputFlags);
    pSDepthUnits->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );

    // AIS Options
    wxStaticBoxSizer* boxAIS = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("AIS")), wxVERTICAL);
    generalSizer->Add(boxAIS, 0, wxALL | wxEXPAND, border_size);
    
    pCBShowAIS = new wxCheckBox(pDisplayPanel, IDCO_SHOW_AIS_CHECKBOX, _("Show AIS targets"));
    boxAIS->Add(pCBShowAIS, verticalInputFlags);
    pCBShowAIS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pCBAttenAIS = new wxCheckBox(pDisplayPanel, IDCO_ATTEN_AIS_CHECKBOX, _("Minimize less critical targets"));
    boxAIS->Add(pCBAttenAIS, verticalInputFlags);
    pCBAttenAIS->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    
    
    // spacer
    generalSizer->Add(0, interGroupSpace);
    
    // Tide/Current Options
    wxStaticBoxSizer* boxTC = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("Tides and Currents")), wxVERTICAL);
    generalSizer->Add(boxTC, 0, wxALL | wxEXPAND, border_size);
    
    pCDOTides = new wxCheckBox(pDisplayPanel, IDCO_TIDES_CHECKBOX, _("Show Tide stations"));
    boxTC->Add(pCDOTides, verticalInputFlags);
    pCDOTides->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pCDOCurrents = new wxCheckBox(pDisplayPanel, IDCO_CURRENTS_CHECKBOX, _("Show Currents"));
    boxTC->Add(pCDOCurrents, verticalInputFlags);
    pCDOCurrents->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    // spacer
    generalSizer->Add(0, interGroupSpace);
 
    // ENC Options
    wxStaticBoxSizer* boxENC = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("Vector Charts")), wxVERTICAL);
    generalSizer->Add(boxENC, 0, wxALL | wxEXPAND, border_size);
    
    pCDOENCText = new wxCheckBox(pDisplayPanel, IDCO_ENCTEXT_CHECKBOX1, _("Show text"));
    boxENC->Add(pCDOENCText, verticalInputFlags);
    pCDOENCText->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );

    pCBENCDepth = new wxCheckBox(pDisplayPanel, IDCO_ENCDEPTH_CHECKBOX1, _("Show depths"));
    boxENC->Add(pCBENCDepth, verticalInputFlags);
    pCBENCDepth->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pCBENCBuoyLabels = new wxCheckBox(pDisplayPanel, IDCO_ENCBUOYLABEL_CHECKBOX1, _("Buoy/Light Labels"));
    boxENC->Add(pCBENCBuoyLabels, verticalInputFlags);
    pCBENCBuoyLabels->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
 
    pCBENCLights = new wxCheckBox(pDisplayPanel, IDCO_ENCBUOYLABEL_CHECKBOX1, _("Lights"));
    boxENC->Add(pCBENCLights, verticalInputFlags);
    pCBENCLights->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
 
    pCBENCLightDesc = new wxCheckBox(pDisplayPanel, IDCO_ENCBUOY_CHECKBOX1, _("Light Descriptions"));
    boxENC->Add(pCBENCLightDesc, verticalInputFlags);
    pCBENCLightDesc->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pCBENCAnchorDetails = new wxCheckBox(pDisplayPanel, IDCO_ENCANCHOR_CHECKBOX1, _("Anchoring Info"));
    boxENC->Add(pCBENCAnchorDetails, verticalInputFlags);
    pCBENCAnchorDetails->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
        // spacer
    boxENC->Add(0, interGroupSpace);

    // display category
    boxENC->Add( new wxStaticText(pDisplayPanel, wxID_ANY, _("Display Category")), verticalInputFlags);
    wxString pDispCatStrings[] = {_("Base"), _("Standard"), _("All"), _("User Standard")};
    m_pDispCat = new wxChoice(pDisplayPanel, ID_CODISPCAT, wxDefaultPosition,  wxDefaultSize, 4, pDispCatStrings);
    boxENC->Add(m_pDispCat, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    m_pDispCat->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
#ifdef __OCPN__ANDROID__
    GetHandle()->setStyleSheet(getAdjustedDialogStyleSheet());
#endif

    RefreshControlValues();
    
    SetAutoLayout( true );
    
    topsizer->Fit( this );
    
}

void CanvasOptions::OnEraseBackground( wxEraseEvent& event )
{
}


void CanvasOptions::OnClose( wxCloseEvent& event )
{
//     SetReturnCode(wxID_CANCEL);
//     EndModal( wxID_CANCEL );
}

void CanvasOptions::OnOptionChange( wxCommandEvent &event)
{
    UpdateCanvasOptions();
}



void CanvasOptions::RefreshControlValues( void )
{
    ChartCanvas *parentCanvas = wxDynamicCast(m_parent, ChartCanvas);
    if(!parentCanvas)
        return;


	//������� ���� ������

    // Control options
//    pCBToolbar->SetValue(parentCanvas->GetToolbarEnable());

    // Navigation Mode
     int mode = parentCanvas->GetUpMode();
     if(mode == NORTH_UP_MODE)
         pCBNorthUp->SetValue( true );
     else if(mode == COURSE_UP_MODE)
         pCBCourseUp->SetValue( true );
     else
         pCBHeadUp->SetValue( true );

	 int weatherMode = parentCanvas->GetWeatherHeightMode();
	 if (weatherMode == WAVE_HEIGHT)
		 pWaveHeight->SetValue(true);
	 else if (weatherMode == RIPPLE_HEIGHT)
		 pRippleHeight->SetValue(true);
	 else
		 pWaveRippleHeight->SetValue(true);

	 double dangerHeight = parentCanvas->GetDangerHeight();
	 pSliderDangerHeight->SetValue(dangerHeight*100);//m->sm


	 std::vector<std::string> dateTimeChoices = parentCanvas->GetDateTimeChoices();
	 pChoiceDateTime->Clear();
	 pChoiceStartTime->Clear();
	 wxString noData("no data");
	 pChoiceDateTime->Append(noData);
	 pChoiceStartTime->Append(noData);
	 for(int i = 0; i < dateTimeChoices.size(); i++) {
		 wxString temp = dateTimeChoices[i];
		 pChoiceDateTime->Append(temp);
		 pChoiceStartTime->Append(temp);
	 }

	 std::string dateTime = parentCanvas->GetDateTime();
	 if (dateTime == "") {
		 pChoiceDateTime->SetSelection(0);
	 }
	 else {
		 int index = pChoiceDateTime->FindString(dateTime);
		 if (index == wxNOT_FOUND) {
			 pChoiceDateTime->SetSelection(0);
		 }
		 else {
			 pChoiceDateTime->SetSelection(index);
		 }
	 }

	 std::string startTime = parentCanvas->GetStartTime();
	 if (startTime == "") {
		 pChoiceStartTime->SetSelection(0);
	 }
	 else {
		 int index = pChoiceStartTime->FindString(startTime);
		 if (index == wxNOT_FOUND) {
			 pChoiceStartTime->SetSelection(0);
		 }
		 else {
			 pChoiceStartTime->SetSelection(index);
		 }
	 }

	pCBDrawWaveHeight->SetValue(parentCanvas->GetDrawWaveHeightEnabled());
	pCBCheckRoute->SetValue(parentCanvas->GetCheckRouteEnabled());
	pCBCalculateRoute->SetValue(parentCanvas->GetCalculateRouteEnabled());
	pCBCalculateFuelRoute->SetValue(parentCanvas->GetCalculateFuelRouteEnabled());
	pCBCheckOptimalRoute->SetValue(parentCanvas->GetCheckOptimalRoute());

	wxDateTime tempThreeHours(wxDateTime((time_t)parentCanvas->GetStartTimeThreeHours()).ToUTC());
	pThreeHoursTime->SetValue(tempThreeHours);

	pShipDangerHeight->SetValue(parentCanvas->GetShipDangerHeight());

	pShipN->SetValue(parentCanvas->GetShipN());

	pShipD->SetValue(parentCanvas->GetShipD());

	pShipL->SetValue(parentCanvas->GetShipL());

	pShipDelta->SetValue(parentCanvas->GetShipDelta());

	pShipSpeed->SetValue(parentCanvas->GetShipSpeed());
	
	pShipDraft->SetValue(parentCanvas->GetShipDraft());
    
    pCBLookAhead->SetValue(parentCanvas->GetLookahead());
    
    //  Display options
    pCDOQuilting->SetValue(parentCanvas->GetQuiltMode());
    pSDisplayGrid->SetValue(parentCanvas->GetShowGrid());
    pCDOOutlines->SetValue(parentCanvas->GetShowOutlines());
    pSDepthUnits->SetValue(parentCanvas->GetShowDepthUnits());
 
    // AIS Options
    pCBShowAIS->SetValue(parentCanvas->GetShowAIS());
    pCBAttenAIS->SetValue(parentCanvas->GetAttenAIS());
    
    // Tide/Current
    pCDOTides->SetValue(parentCanvas->GetbShowTide());
    pCDOCurrents->SetValue(parentCanvas->GetbShowCurrent());;
    
    //ENC Options
    pCDOENCText->SetValue(parentCanvas->GetShowENCText());
    pCBENCDepth->SetValue(parentCanvas->GetShowENCDepth());
    pCBENCLightDesc->SetValue(parentCanvas->GetShowENCLightDesc());
    pCBENCBuoyLabels->SetValue(parentCanvas->GetShowENCBuoyLabels());
    pCBENCLights->SetValue(parentCanvas->GetShowENCLights());
    pCBENCAnchorDetails->SetValue(parentCanvas->GetShowENCAnchor());

    //pCBENCLightDesc->Enable(parentCanvas->GetShowENCLights());
    
    
    //  Display category
    int nset = 2;  // default OTHER
    switch (parentCanvas->GetENCDisplayCategory()) {
        case (DISPLAYBASE):
            nset = 0;
            break;
        case (STANDARD):
            nset = 1;
            break;
        case (OTHER):
            nset = 2;
            break;
        case (MARINERS_STANDARD):
            nset = 3;
            break;
        default:
            nset = 3;
            break;
    }
    m_pDispCat->SetSelection(nset);
    
    // If no ENCs are available in the current canvas group, then disable the ENC related options.
    pCDOENCText->Enable(m_ENCAvail);
    pCBENCDepth->Enable(m_ENCAvail);
    pCBENCLightDesc->Enable(m_ENCAvail && parentCanvas->GetShowENCLights());
    pCBENCBuoyLabels->Enable(m_ENCAvail);
    pCBENCLights->Enable(m_ENCAvail);
    
    //  Anchor conditions are only available if display category is "All" or "User Standard"
    pCBENCAnchorDetails->Enable(m_ENCAvail && (nset > 1));  

    //  Many options are not valid if display category is "Base"
    if(nset == 0){
        pCDOENCText->Disable();
        pCBENCDepth->Disable();
        pCBENCLightDesc->Disable();
        pCBENCBuoyLabels->Disable();
        pCBENCLights->Disable();
    }
        
    m_pDispCat->Enable(m_ENCAvail);
    
}

void CanvasOptions::SetENCAvailable( bool avail )
{ 
    m_ENCAvail = avail;
    RefreshControlValues();
}

void CanvasOptions::UpdateCanvasOptions( void )
{
    ChartCanvas *parentCanvas = wxDynamicCast(m_parent, ChartCanvas);
    if(!parentCanvas)
        return;
    
    bool b_needRefresh = false;
    bool b_needReLoad = false;
    
//     if(pCBToolbar->GetValue() != parentCanvas->GetToolbarEnable()){
//         parentCanvas->SetToolbarEnable( pCBToolbar->GetValue() );
//         b_needRefresh = true;
//     }
    
    if(pCDOQuilting->GetValue() != parentCanvas->GetQuiltMode()){
        parentCanvas->ToggleCanvasQuiltMode();
    }
    
    if(pSDisplayGrid->GetValue() != parentCanvas->GetShowGrid()){
        parentCanvas->SetShowGrid(pSDisplayGrid->GetValue());
        b_needRefresh = true;
    }
    
    if(pCDOOutlines->GetValue() != parentCanvas->GetShowOutlines()){
        parentCanvas->SetShowOutlines(pCDOOutlines->GetValue());
        b_needRefresh = true;
    }
    if(pSDepthUnits->GetValue() != parentCanvas->GetShowDepthUnits()){
        parentCanvas->SetShowDepthUnits(pSDepthUnits->GetValue());
        b_needRefresh = true;
    }

    if(pCBShowAIS->GetValue() != parentCanvas->GetShowAIS()){
        parentCanvas->SetShowAIS(pCBShowAIS->GetValue());
        b_needRefresh = true;
    }
    
    if(pCBAttenAIS->GetValue() != parentCanvas->GetAttenAIS()){
        parentCanvas->SetAttenAIS(pCBAttenAIS->GetValue());
        b_needRefresh = true;
    }
    
    if(pCDOTides->GetValue() != parentCanvas->GetbShowTide()){
        parentCanvas->ShowTides(pCDOTides->GetValue());
        b_needRefresh = true;
    }
    if(pCDOCurrents->GetValue() != parentCanvas->GetbShowCurrent()){
        parentCanvas->ShowCurrents(pCDOCurrents->GetValue());
        b_needRefresh = true;
    }

    //  ENC Options
    if(pCDOENCText->GetValue() != parentCanvas->GetShowENCText()){
        parentCanvas->SetShowENCText(pCDOENCText->GetValue());
        b_needReLoad = true;
    }

    if(pCBENCDepth->GetValue() != parentCanvas->GetShowENCDepth()){
        parentCanvas->SetShowENCDepth(pCBENCDepth->GetValue());
        b_needReLoad = true;
    }
    
    if(pCBENCLightDesc->GetValue() != parentCanvas->GetShowENCLightDesc()){
        parentCanvas->SetShowENCLightDesc(pCBENCLightDesc->GetValue());
        b_needReLoad = true;
    }
    
    if(pCBENCBuoyLabels->GetValue() != parentCanvas->GetShowENCBuoyLabels()){
        parentCanvas->SetShowENCBuoyLabels(pCBENCBuoyLabels->GetValue());
        b_needReLoad = true;
    }

    if(pCBENCLights->GetValue() != parentCanvas->GetShowENCLights()){
        parentCanvas->SetShowENCLights(pCBENCLights->GetValue());
        b_needReLoad = true;
    }
    
    if(pCBENCAnchorDetails->GetValue() != parentCanvas->GetShowENCAnchor()){
        parentCanvas->SetShowENCAnchor(pCBENCAnchorDetails->GetValue());
        b_needReLoad = true;
    }
    
    int newMode = NORTH_UP_MODE;
    if(pCBCourseUp->GetValue())
        newMode = COURSE_UP_MODE;
    else if(pCBHeadUp->GetValue())
        newMode = HEAD_UP_MODE;
    
    if(newMode != parentCanvas->GetUpMode()){
        parentCanvas->SetUpMode(newMode);
        b_needReLoad = true;
    }

	int newHeightMode = WAVE_HEIGHT;
	if (pRippleHeight->GetValue()) {
		newHeightMode = RIPPLE_HEIGHT;
	}
	else if (pWaveRippleHeight->GetValue()) {
		newHeightMode = WAVE_RIPPLE_HEIGHT;
	}

	if (newHeightMode != parentCanvas->GetWeatherHeightMode()) {
		parentCanvas->SetWeatherHeightMode(newHeightMode);
		b_needReLoad = true;
	}

	double newDangerHeight = double (pSliderDangerHeight->GetValue())/100;
	if (newDangerHeight != parentCanvas->GetDangerHeight()) {
		parentCanvas->SetDangerHeight(newDangerHeight);
		b_needReLoad = true;
	}

	wxString temp(pChoiceDateTime->GetString(pChoiceDateTime->GetSelection()));
	if (temp.ToStdString() != parentCanvas->GetDateTime()) {
		parentCanvas->SetDateTime(temp.ToStdString());
		b_needReLoad = true;
	}

	wxString tempStart(pChoiceStartTime->GetString(pChoiceStartTime->GetSelection()));
	if (tempStart.ToStdString() != parentCanvas->GetStartTime()) {
		parentCanvas->SetStartTime(tempStart.ToStdString());
		//b_needReLoad = true;
	}

	if (pCBDrawWaveHeight->GetValue() != parentCanvas->GetDrawWaveHeightEnabled()) {
		parentCanvas->SetDrawWaveHeightEnabled(!parentCanvas->GetDrawWaveHeightEnabled());
		b_needReLoad = true;
	}

	if (pCBCheckRoute->GetValue() != parentCanvas->GetCheckRouteEnabled()) {
		parentCanvas->SetCheckRouteEnabled(!parentCanvas->GetCheckRouteEnabled());
		b_needReLoad = true;
	}

	if (pCBCalculateRoute->GetValue() != parentCanvas->GetCalculateRouteEnabled()) {
		parentCanvas->SetCalculateRouteEnabled(!parentCanvas->GetCalculateRouteEnabled());
		b_needReLoad = true;
	}
	if (pCBCalculateFuelRoute->GetValue() != parentCanvas->GetCalculateFuelRouteEnabled()) {
		parentCanvas->SetCalculateFuelRouteEnabled(!parentCanvas->GetCalculateFuelRouteEnabled());
		b_needReLoad = true;
	}
	if (pCBCheckOptimalRoute->GetValue() != parentCanvas->GetCheckOptimalRoute()) {
		parentCanvas->SetCheckOptimalRouteEnabled(!parentCanvas->GetCheckOptimalRoute());
		b_needReLoad = true;
	}
	//wxDateTime((time_t)g_track_rotate_time).ToUTC()
	int h, m, s;
	pThreeHoursTime->GetTime(&h, &m, &s);
	int tempThreeHours = h * 3600 + m * 60 + s;
	if (tempThreeHours != parentCanvas->GetStartTimeThreeHours()) {
		parentCanvas->SetStartTimeThreeHours(tempThreeHours);
	}
		
	if (pShipDangerHeight->GetValue() != parentCanvas->GetShipDangerHeight()) {
		parentCanvas->SetShipDangerHeight(pShipDangerHeight->GetValue());
	}

	if (pShipN->GetValue() != parentCanvas->GetShipN()) {
		parentCanvas->SetShipN(pShipN->GetValue());
	}

	if (pShipD->GetValue() != parentCanvas->GetShipD()) {
		parentCanvas->SetShipD(pShipD->GetValue());
	}

	if (pShipL->GetValue() != parentCanvas->GetShipL()) {
		parentCanvas->SetShipL(pShipL->GetValue());
	}

	if (pShipDelta->GetValue() != parentCanvas->GetShipDelta()) {
		parentCanvas->SetShipDelta(pShipDelta->GetValue());
	}

	if (pShipSpeed->GetValue() != parentCanvas->GetShipSpeed()) {
		parentCanvas->SetShipSpeed(pShipSpeed->GetValue());
	}

	if (pShipDraft->GetValue() != parentCanvas->GetShipDraft()) {
		parentCanvas->SetShipDraft(pShipDraft->GetValue());
	}

    if(pCBLookAhead->GetValue() != parentCanvas->GetLookahead()){
        parentCanvas->ToggleLookahead();
        parentCanvas->UpdateFollowButtonState();
        b_needReLoad = true;
    }
    
    int nset = 2;
    switch (parentCanvas->GetENCDisplayCategory()) {
        case (DISPLAYBASE): nset = 0; break;
        case (STANDARD): nset = 1; break;
        case (OTHER): nset = 2; break;
        case (MARINERS_STANDARD): nset = 3; break;
        default: nset = 2; break;
    }
    
    
    if(m_pDispCat->GetSelection() != nset){
        int valSet = STANDARD;
        int newSet = m_pDispCat->GetSelection();
        switch(newSet){
            case 0: valSet = DISPLAYBASE; break;
            case 1: valSet = STANDARD; break;
            case 2: valSet = OTHER; break;
            case 3: valSet = MARINERS_STANDARD; break;
            default: valSet = STANDARD; break;
        }
        parentCanvas->SetENCDisplayCategory( valSet);
        b_needReLoad = true;
        
        //  Anchor conditions are only available if display category is "All" or "User Standard"
        pCBENCAnchorDetails->Enable(newSet > 1);

    }
    
    if(b_needReLoad){
        parentCanvas->ReloadVP();
    }
    else if (b_needRefresh){
        parentCanvas->Refresh(true);
        parentCanvas->InvalidateGL();
    }
    
    RefreshControlValues();
        
}



