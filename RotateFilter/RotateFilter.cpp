//------------------------------------------------------------------------------
// File: RotateFilter.cpp
//
// Desc: Rotate YUY2 image filter.
//
//------------------------------------------------------------------------------


//
//
// Base classes used
//
// CTransformFilter     A transform filter with one input and output pin
// CPersistStream       Handles the grunge of supporting IPersistStream
//
//


#include <windows.h>
#include <streams.h>
#include <initguid.h>

#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

#include "guids.h"
#include "irf.h"
#include "Props.h"
#include "RotateFilter.h"
#include "resource.h"
// Setup information

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudpPins[] =
{
    { L"Input",             // Pins string name
      FALSE,                // Is it rendered
      FALSE,                // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      1,                    // Number of types
      &sudPinTypes          // Pin information
    },
    { L"Output",            // Pins string name
      FALSE,                // Is it rendered
      TRUE,                 // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      1,                    // Number of types
      &sudPinTypes          // Pin information
    }
};

const AMOVIESETUP_FILTER sudRotateFilter =
{
    &CLSID_RotateFilter,    // Filter CLSID
    L"Rotate Filter (YUY2)",// String name
    MERIT_DO_NOT_USE,       // Filter merit
    2,                      // Number of pins
    sudpPins                // Pin information
};


// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

CFactoryTemplate g_Templates[] = {
    { L"Rotate Filter"
    , &CLSID_RotateFilter
    , RotateFilter::CreateInstance
    , NULL
    , &sudRotateFilter }
  ,
    { L"Rotate direction"
    , &CLSID_RotateFilterPropertyPage
    , RotateFilterProperties::CreateInstance }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

//
// DllRegisterServer
//
// Handles sample registry and unregistry
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

} // DllUnregisterServer


//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


//
// Constructor
//
RotateFilter::RotateFilter(TCHAR *tszName,
                   LPUNKNOWN punk,
                   HRESULT *phr) :
    CTransformFilter(tszName, punk, CLSID_RotateFilter),
    m_change(0),
    m_effect(IDC_DM0),
    m_lBufferRequest(1),
    CPersistStream(punk, phr)
{
} // (Constructor)


//
// CreateInstance
//
// Provide the way for COM to create a EZrgb24 object
//
CUnknown *RotateFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    ASSERT(phr);
    
    RotateFilter *pNewObject = new RotateFilter(NAME("Rotate Filter"), punk, phr);

    if (pNewObject == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }
    return pNewObject;

} // CreateInstance


//
// NonDelegatingQueryInterface
//
// Reveals IIPEffect and ISpecifyPropertyPages
//
STDMETHODIMP RotateFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IIPEffect) {
        return GetInterface((IIPEffect *) this, ppv);

    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);

    } else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

} // NonDelegatingQueryInterface


//
// Transform
//
HRESULT RotateFilter::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    CheckPointer(pIn,E_POINTER);   
    CheckPointer(pOut,E_POINTER);   

    // Copy the properties across

    HRESULT hr = Copy(pIn, pOut);
    if (FAILED(hr)) {
        return hr;
    }

    // Check to see if it is time to do the sample
    return RotateTransform(pIn, pOut);

} // Transform


//
// Copy
//
// Make destination an identical copy of source
//
HRESULT RotateFilter::Copy(IMediaSample *pSource, IMediaSample *pDest) const
{
    CheckPointer(pSource, E_POINTER);
    CheckPointer(pDest, E_POINTER);

    // Copy the sample data

    BYTE *pSourceBuffer, *pDestBuffer;
    long lSourceSize = pSource->GetActualDataLength();

#ifdef DEBUG
    long lDestSize = pDest->GetSize();
    ASSERT(lDestSize >= lSourceSize);
#endif

    pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);

    CopyMemory((PVOID)pDestBuffer, (PVOID)pSourceBuffer, lSourceSize);

    // Copy the sample times

    REFERENCE_TIME TimeStart, TimeEnd;
    if (NOERROR == pSource->GetTime(&TimeStart, &TimeEnd)) {
        pDest->SetTime(&TimeStart, &TimeEnd);
    }

    LONGLONG MediaStart, MediaEnd;
    if (pSource->GetMediaTime(&MediaStart, &MediaEnd) == NOERROR) {
        pDest->SetMediaTime(&MediaStart, &MediaEnd);
    }

    // Copy the Sync point property

    HRESULT hr = pSource->IsSyncPoint();
    if (hr == S_OK) {
        pDest->SetSyncPoint(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetSyncPoint(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the media type
    AM_MEDIA_TYPE *pMediaType;
    pSource->GetMediaType(&pMediaType);
    if ((m_effect == IDC_DM90) || (m_effect == IDC_DM270))
    {
        AM_MEDIA_TYPE* pT = CreateMediaType(&m_pInput->CurrentMediaType());
        if (pT != NULL)
        {
            ULONG height = ((VIDEOINFOHEADER *)pT->pbFormat)->bmiHeader.biHeight;
            ULONG width = ((VIDEOINFOHEADER *)pT->pbFormat)->bmiHeader.biWidth;
            ((VIDEOINFOHEADER *)pT->pbFormat)->bmiHeader.biHeight = width;
            ((VIDEOINFOHEADER *)pT->pbFormat)->bmiHeader.biWidth = height;
            pDest->SetMediaType(pT);
            FreeMediaType(*pT);
        }
    }
    else
        pDest->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);

    // Copy the preroll property

    hr = pSource->IsPreroll();
    if (hr == S_OK) {
        pDest->SetPreroll(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetPreroll(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the discontinuity property

    hr = pSource->IsDiscontinuity();
    if (hr == S_OK) {
    pDest->SetDiscontinuity(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetDiscontinuity(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the actual data length

    long lDataLength = pSource->GetActualDataLength();
    pDest->SetActualDataLength(lDataLength);

    return NOERROR;

} // Copy

//
// RotateTransform
//
HRESULT RotateFilter::RotateTransform(IMediaSample *pSrc, IMediaSample *pDest)
{
    BYTE* pInputBuffer;         // Pointer to the actual image buffer
    BYTE* pOutputBuffer;        //
    long lDataLen;              // Holds length of any given sample

    AM_MEDIA_TYPE* pType = &m_pInput->CurrentMediaType();
    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pType->pbFormat;
    ASSERT(pvi);

    CheckPointer(pSrc,E_POINTER);
    CheckPointer(pDest, E_POINTER);
    pSrc->GetPointer(&pInputBuffer);
    lDataLen = pSrc->GetSize();
    pDest->GetPointer(&pOutputBuffer);

    // Get the image properties from the BITMAPINFOHEADER
    switch (m_effect)
    {
        default:
        case IDC_DM0: break;
        // Zero out the green and blue components to leave only the red
        // so acting as a filter - for better visual results, compute a
        // greyscale value for the pixel and make that the red component

        case IDC_DM90:                        
        {
            BYTE *pSurBuf = pInputBuffer;
            BYTE *pDstBuf = pOutputBuffer;
            const ULONG ulHeight = pvi->bmiHeader.biHeight;
            const ULONG ulWidth = pvi->bmiHeader.biWidth;
            const ULONG iNewWidth = ulHeight;
            const ULONG iNewHeight = ulWidth;
            ULONG i, j;

            // Clear target memory buffer to all black
            {
                pDstBuf = pOutputBuffer;
                for (j = 0; j<ulHeight; j++)
                {
                    for (i = 0; i<ulWidth / 2; i++)
                    {
                        *(pDstBuf + i * 4) = 0x00;        // Yn
                        *(pDstBuf + 1 + i * 4) = 0x80;    // Un
                        *(pDstBuf + 2 + i * 4) = 0x00;    // Yn+1
                        *(pDstBuf + 3 + i * 4) = 0x80;    // Vn
                    }
                    pDstBuf += ulWidth * 2;
                }
            }

            // 2 columns at a time
            //  YU YV
            //  YU YV
            //  YU YV
            //  YU YV
            for (j = 0; j < iNewWidth; j += 2) // 2 Y at a time
            {
                pSurBuf = pInputBuffer + j*iNewHeight * 2;
                pDstBuf = pOutputBuffer + 2 * (iNewWidth - j) - 2; // rotate, point to the right YV

                for (i = 0; i < iNewHeight; i += 2)
                { // for V, copy V, skip U,
                    pDstBuf[(i)*iNewWidth * 2] = pSurBuf[2 * i]; // Y
                    pDstBuf[(i)*iNewWidth * 2 + 1] = pSurBuf[2 * (i + 1) + 1]; //V
                    pDstBuf[(i + 1)*iNewWidth * 2] = pSurBuf[2 * (i + 1)]; //Y
                    pDstBuf[(i + 1)*iNewWidth * 2 + 1] = pSurBuf[2 * (i + 1) + 1]; //V
                }
                pSurBuf = pInputBuffer + (j + 1)*iNewHeight * 2;
                pDstBuf = pOutputBuffer + 2 * (iNewWidth - j) - 4; // rotate, point to the right YU

                for (i = 0; i < iNewHeight; i += 2)
                { // for U, copy U, skip V
                    pDstBuf[(i)*iNewWidth * 2] = pSurBuf[2 * i]; // Y
                    pDstBuf[(i)*iNewWidth * 2 + 1] = pSurBuf[2 * i + 1]; //U
                    pDstBuf[(i + 1)*iNewWidth * 2] = pSurBuf[2 * (i + 1)]; //Y
                    pDstBuf[(i + 1)*iNewWidth * 2 + 1] = pSurBuf[2 * i + 1]; //U
                }
            }
        }
            break;
    
        case IDC_DM180:
        {
            BYTE *pSurBuf = pInputBuffer;
            BYTE *pDstBuf = pOutputBuffer;
            ULONG ulHeight = pvi->bmiHeader.biHeight;;
            ULONG ulWidth = pvi->bmiHeader.biWidth;

            int iTempX, iTempY;
            UCHAR ucTemp;

            pSurBuf = pInputBuffer;
            pDstBuf = pOutputBuffer;
            pDstBuf += ulHeight*ulWidth * 2 - 4;
            for (iTempY = 0; iTempY<(int)ulHeight; iTempY++)
            {
                for (iTempX = 0; iTempX<(int)ulWidth / 2; iTempX++)
                {
                    CopyMemory((PUCHAR)pDstBuf, (PUCHAR)pSurBuf, 4);
                    ucTemp = *(PUCHAR)pDstBuf;
                    *(PUCHAR)pDstBuf = *(PUCHAR)(pDstBuf + 2);
                    *(PUCHAR)(pDstBuf + 2) = ucTemp;
                    pSurBuf += 4;
                    pDstBuf -= 4;
                }
            }
        }
            break;

        case IDC_DM270:
        {
            BYTE *pSurBuf = pInputBuffer;
            BYTE *pDstBuf = pOutputBuffer;
            ULONG ulHeight = pvi->bmiHeader.biHeight;
            ULONG ulWidth = pvi->bmiHeader.biWidth;
            ULONG iNewWidth = ulHeight;
            ULONG iNewHeight = ulWidth;
            ULONG i, j;

            {
                pDstBuf = pOutputBuffer;
                for (j = 0; j<ulHeight; j++)
                {
                    for (i = 0; i<ulWidth / 2; i++)
                    {
                        *(pDstBuf + i * 4) = 0x00;
                        *(pDstBuf + 1 + i * 4) = 0x80;
                        *(pDstBuf + 2 + i * 4) = 0x00;
                        *(pDstBuf + 3 + i * 4) = 0x80;
                    }
                    pDstBuf += ulWidth * 2;
                }
            }

            // 2 columns at a time
            //  YU YV
            //  YU YV
            //  YU YV
            //  YU YV
            for (j = 0; j < iNewWidth; j += 2) // 2 Y at a time
            {
                pSurBuf = pInputBuffer + j*(iNewHeight * 2);
                pDstBuf = pOutputBuffer + (2 * iNewWidth)*(iNewHeight - 1) + 2 * j; // rotate, point to the bottom YU

                for (i = 0; i < iNewHeight; i += 2)
                { // for U, copy U, skip V,
                    *(pDstBuf - (i*iNewWidth * 2)) = pSurBuf[2 * i]; // Y
                    *(pDstBuf - (i*iNewWidth * 2) + 1) = pSurBuf[2 * i + 1]; //U
                    *(pDstBuf - ((i + 1)*iNewWidth * 2)) = pSurBuf[2 * (i + 1)]; //Y
                    *(pDstBuf - ((i + 1)*iNewWidth * 2) + 1) = pSurBuf[2 * i + 1]; //U
                }

                pSurBuf = pInputBuffer + (j + 1)*iNewHeight * 2;
                pDstBuf = pOutputBuffer + (2 * iNewWidth)*(iNewHeight - 1) + 2 * j + 2;  // rotate, point to the bottom YV

                for (i = 0; i < iNewHeight; i += 2)
                { // for V, copy V, skip U
                    *(pDstBuf - (i*iNewWidth * 2)) = pSurBuf[2 * i]; // Y
                    *(pDstBuf - (i*iNewWidth * 2) + 1) = pSurBuf[2 * (i + 1) + 1]; //V
                    *(pDstBuf - ((i + 1)*iNewWidth * 2)) = pSurBuf[2 * (i + 1)]; //Y
                    *(pDstBuf - ((i + 1)*iNewWidth * 2) + 1) = pSurBuf[2 * (i + 1) + 1]; //V
                }
            }
        }
            break;
        case IDC_HFLIP: // Horizon
        {
            BYTE *pSurBuf = pInputBuffer;
            BYTE *pDstBuf = pOutputBuffer;
            ULONG ulHeight = pvi->bmiHeader.biHeight;
            ULONG ulWidth = pvi->bmiHeader.biWidth;
            for (unsigned i = 0; i < ulHeight; i++)
            {
                pSurBuf = pInputBuffer + (ulWidth * (i + 1) * 2) - 4;
                pDstBuf = pOutputBuffer + (ulWidth * i * 2);
                for (unsigned j = 0; j < (ulWidth * 2); j+=4)
                {
                    pDstBuf[j + 0] = pSurBuf[2 - j];  // Y
                    pDstBuf[j + 1] = pSurBuf[1 - j];  // U
                    pDstBuf[j + 2] = pSurBuf[0 - j];  // Y
                    pDstBuf[j + 3] = pSurBuf[3 - j];  // V
                }
            }
        }
            break;
        case IDC_VFLIP:
        {
            BYTE *pSurBuf = pInputBuffer;
            BYTE *pDstBuf = pOutputBuffer;
            ULONG ulHeight = pvi->bmiHeader.biHeight;
            ULONG ulWidth = pvi->bmiHeader.biWidth;
            for (unsigned i = 0; i < ulHeight; i++)
            {
                pSurBuf = pInputBuffer + (ulHeight - i - 1) * ulWidth * 2; // from bottom to top
                pDstBuf = pOutputBuffer + (ulWidth * i * 2); // from top to bottom
                CopyMemory(pDstBuf, pSurBuf, ulWidth * 2);
            }
        }
            break;
    }

    return NOERROR;
} // Transform (in place)


// Check the input type is OK - return an error otherwise

HRESULT RotateFilter::CheckInputType(const CMediaType *mtIn)
{
    CheckPointer(mtIn,E_POINTER);

    // check this is a VIDEOINFOHEADER type

    if (*mtIn->FormatType() != FORMAT_VideoInfo) {
        return E_INVALIDARG;
    }

    // Can we transform this type

    if (CanPerformYUY2(mtIn)) {
        return NOERROR;
    }
    return E_FAIL;
}


//
// Checktransform
//
// Check a transform can be done between these formats
//
HRESULT RotateFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    CheckPointer(mtIn,E_POINTER);
    CheckPointer(mtOut,E_POINTER);

    if (CanPerformYUY2(mtIn)) 
    {
        return NOERROR;
    }

    return E_FAIL;

} // CheckTransform


//
// DecideBufferSize
//
// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
//
HRESULT RotateFilter::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    // Is the input pin connected

    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = m_pInput->CurrentMediaType().GetSampleSize();
    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT( Actual.cBuffers == 1 );

    if (pProperties->cBuffers > Actual.cBuffers ||
            pProperties->cbBuffer > Actual.cbBuffer) {
                return E_FAIL;
    }
    return NOERROR;

} // DecideBufferSize


//
// GetMediaType
//
// I support one type, namely the type of the input pin
// This type is only available if my input is connected
//
HRESULT RotateFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    // Is the input pin connected

    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    // This should never happen

    if (iPosition < 0) {
        return E_INVALIDARG;
    }

    // Do we have more items to offer

    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    CheckPointer(pMediaType,E_POINTER);
    
    switch (m_effect) {
    default:
    case IDC_DM0:
    case IDC_DM180:
    case IDC_HFLIP:
    case IDC_VFLIP:
        *pMediaType = m_pInput->CurrentMediaType();
        return NOERROR;
    case IDC_DM90:
    case IDC_DM270:
        AM_MEDIA_TYPE* pT = CreateMediaType(&m_pInput->CurrentMediaType());
        if (pT != NULL)
        {
            ULONG height = ((VIDEOINFOHEADER *)pT->pbFormat)->bmiHeader.biHeight;
            ULONG width = ((VIDEOINFOHEADER *)pT->pbFormat)->bmiHeader.biWidth;
            ((VIDEOINFOHEADER *)pT->pbFormat)->bmiHeader.biHeight = width;
            ((VIDEOINFOHEADER *)pT->pbFormat)->bmiHeader.biWidth = height;
            *pMediaType = *pT;
            FreeMediaType(*pT);
            return NOERROR;
        }
        break;
    }

    return E_UNEXPECTED;
} // GetMediaType


//
// CanPerformYUY2
//
// Check if this is a YUY2 format
//
BOOL RotateFilter::CanPerformYUY2(const CMediaType *pMediaType) const
{
    CheckPointer(pMediaType,FALSE);

    if (IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video)) 
    {
        if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_YUY2))
        {
            return TRUE;
        }
    }

    return FALSE;

} // CanPerformYUY2


#define WRITEOUT(var)  hr = pStream->Write(&var, sizeof(var), NULL); \
               if (FAILED(hr)) return hr;

#define READIN(var)    hr = pStream->Read(&var, sizeof(var), NULL); \
               if (FAILED(hr)) return hr;


//
// GetClassID
//
// This is the only method of IPersist
//
STDMETHODIMP RotateFilter::GetClassID(CLSID *pClsid)
{
    return CBaseFilter::GetClassID(pClsid);

} // GetClassID


//
// ScribbleToStream
//
// Overriden to write our state into a stream
//
HRESULT RotateFilter::ScribbleToStream(IStream *pStream)
{
    HRESULT hr;

    WRITEOUT(m_effect);

    return NOERROR;

} // ScribbleToStream


//
// ReadFromStream
//
// Likewise overriden to restore our state from a stream
//
HRESULT RotateFilter::ReadFromStream(IStream *pStream)
{
    HRESULT hr;

    READIN(m_effect);

    return NOERROR;

} // ReadFromStream


//
// GetPages
//
// Returns the clsid's of the property pages we support
//
STDMETHODIMP RotateFilter::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    *(pPages->pElems) = CLSID_RotateFilterPropertyPage;
    return NOERROR;

} // GetPages


//
// get_IPEffect
//
// Return the current effect selected
//
STDMETHODIMP RotateFilter::get_IPEffect(int *IPEffect)
{
    CAutoLock cAutolock(&m_Lock);
    CheckPointer(IPEffect,E_POINTER);

    *IPEffect = m_effect;

    return NOERROR;

} // get_IPEffect


//
// put_IPEffect
//
// Set the required video effect
//
STDMETHODIMP RotateFilter::put_IPEffect(int IPEffect)
{
    CAutoLock cAutolock(&m_Lock);

    m_effect = IPEffect;
    SetDirty(TRUE);
    return NOERROR;
} // put_IPEffect


