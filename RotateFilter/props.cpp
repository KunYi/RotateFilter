//------------------------------------------------------------------------------
// File: Props.cpp
//
// Desc: implementation of RotateFilterProperties class.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include <streams.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"
#include "guids.h"
#include "irf.h"
#include "RotateFilter.h"
#include "Props.h"


//
// CreateInstance
//
// Used by the DirectShow base classes to create instances
//
CUnknown *RotateFilterProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);

    CUnknown *punk = new RotateFilterProperties(lpunk, phr);

    if (punk == NULL) {
        if (phr)
        	*phr = E_OUTOFMEMORY;
    }

    return punk;

} // CreateInstance


//
// Constructor
//
RotateFilterProperties::RotateFilterProperties(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage(NAME("Rotate Property Page"), pUnk,
                      IDD_RotateProp, IDS_TITLE),
    m_pIPEffect(NULL),
    m_bIsInitialized(FALSE)
{
    ASSERT(phr);

} // (Constructor)


//
// OnReceiveMessage
//
// Handles the messages for our property window
//
INT_PTR RotateFilterProperties::OnReceiveMessage(HWND hwnd,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            if (m_bIsInitialized)
            {
                m_bDirty = TRUE;
                if (m_pPageSite)
                {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }
            }
            return (LRESULT) 1;
        }

    }

    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

} // OnReceiveMessage


//
// OnConnect
//
// Called when we connect to a transform filter
//
HRESULT RotateFilterProperties::OnConnect(IUnknown *pUnknown)
{
    CheckPointer(pUnknown,E_POINTER);
    ASSERT(m_pIPEffect == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_IIPEffect, (void **) &m_pIPEffect);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    // Get the initial image FX property
    CheckPointer(m_pIPEffect,E_FAIL);
    m_pIPEffect->get_IPEffect(&m_effect);

    m_bIsInitialized = FALSE ;
    return NOERROR;

} // OnConnect


//
// OnDisconnect
//
// Likewise called when we disconnect from a filter
//
HRESULT RotateFilterProperties::OnDisconnect()
{
    // Release of Interface after setting the appropriate old effect value
    if(m_pIPEffect)
    {
        m_pIPEffect->Release();
        m_pIPEffect = NULL;
    }
    return NOERROR;

} // OnDisconnect


//
// OnActivate
//
// We are being activated
//
HRESULT RotateFilterProperties::OnActivate()
{

   CheckRadioButton(m_Dlg, IDC_DM0, IDC_VFLIP, m_effect);
    m_bIsInitialized = TRUE;

    return NOERROR;

} // OnActivate


//
// OnDeactivate
//
// We are being deactivated
//
HRESULT RotateFilterProperties::OnDeactivate(void)
{
    ASSERT(m_pIPEffect);

    m_bIsInitialized = FALSE;
    GetControlValues();

    return NOERROR;

} // OnDeactivate


//
// OnApplyChanges
//
// Apply any changes so far made
//
HRESULT RotateFilterProperties::OnApplyChanges()
{
    GetControlValues();

    CheckPointer(m_pIPEffect,E_POINTER);
    m_pIPEffect->put_IPEffect(m_effect);

    return NOERROR;

} // OnApplyChanges


void RotateFilterProperties::GetControlValues()
{
    // Find which special effect we have selected

    for (int i = IDC_DM0; i <= IDC_VFLIP; i++) 
    {
        if (IsDlgButtonChecked(m_Dlg, i)) 
        {
            m_effect = i;
            break;
        }
    }
}
