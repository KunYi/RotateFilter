//------------------------------------------------------------------------------
// File: RotateFilter.h
//------------------------------------------------------------------------------


class RotateFilter : public CTransformFilter,
         public IIPEffect,
         public ISpecifyPropertyPages,
         public CPersistStream
{

public:

    DECLARE_IUNKNOWN;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    // Reveals IRotateFilter and ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // CPersistStream stuff
    HRESULT ScribbleToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);

    // Overrriden from CTransformFilter base class

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

    // These implement the custom IIPEffect interface

    STDMETHODIMP get_IPEffect(int *IPEffect);
    STDMETHODIMP put_IPEffect(int IPEffect);

    // ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID *pPages);

    // CPersistStream override
    STDMETHODIMP GetClassID(CLSID *pClsid);

private:

    // Constructor
    RotateFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

    // Look after doing the special effect
    BOOL CanPerformYUY2(const CMediaType *pMediaType) const;
    HRESULT Copy(IMediaSample *pSource, IMediaSample *pDest) const;
    HRESULT RotateTransform(IMediaSample *pIn, IMediaSample *pOut);

    CCritSec    m_Lock;          // Private play critical section
    int         m_effect;               // Which effect are we processing
    int         m_change;
    const long m_lBufferRequest;        // The number of buffers to use

    // helper function
    void clearDstBuff(PBYTE dst, UINT32 width, UINT32 height);

}; // RotateFilter

