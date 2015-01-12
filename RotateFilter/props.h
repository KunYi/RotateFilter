//------------------------------------------------------------------------------
// File: Props.h
//
// Desc: definition of RotateFilterProperties class.
//
//------------------------------------------------------------------------------

#include <strsafe.h>

class RotateFilterProperties : public CBasePropertyPage
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

private:

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void    GetControlValues();

    RotateFilterProperties(LPUNKNOWN lpunk, HRESULT *phr);

    BOOL m_bIsInitialized;      // Used to ignore startup messages
    int m_effect;               // Which effect are we processing
    IIPEffect *m_pIPEffect;     // The custom interface on the filter

}; // RotateFilterProperties

