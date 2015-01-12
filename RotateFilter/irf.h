//------------------------------------------------------------------------------
// File: IRF.h
//
// Desc: custom interface to allow the user to
//       perform image rotate.
//------------------------------------------------------------------------------


#ifndef __IRF__
#define __IRF__

#ifdef __cplusplus
extern "C" {
#endif

    // {4EC83921-05D4-41E2-B31F-5C5E56B5E786}
    DEFINE_GUID(IID_IIPEffect,
        0x4ec83921, 0x5d4, 0x41e2, 0xb3, 0x1f, 0x5c, 0x5e, 0x56, 0xb5, 0xe7, 0x86);

    DECLARE_INTERFACE_(IIPEffect, IUnknown)
    {
        STDMETHOD(get_IPEffect) (THIS_
                    int *effectNum         // The current effect
                 ) PURE;

        STDMETHOD(put_IPEffect) (THIS_
                    int effectNum          // Change to this effect
                 ) PURE;
    };

#ifdef __cplusplus
}
#endif

#endif // __IRF__

