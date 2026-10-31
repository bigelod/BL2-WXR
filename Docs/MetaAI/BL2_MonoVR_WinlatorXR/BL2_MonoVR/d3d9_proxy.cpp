#include "pch.h"
#include "d3d9_proxy.h"
#include "WinXrApi.h"

extern WinXrApi* g_pWinXrApi;
HMODULE g_hRealD3D9 = nullptr;
PFN_Direct3DCreate9 g_pfnRealDirect3DCreate9 = nullptr;

ProxyIDirect3D9::ProxyIDirect3D9(IDirect3D9* real) : m_real(real) {}

STDMETHODIMP ProxyIDirect3D9::QueryInterface(REFIID riid, void** ppvObj) { return m_real->QueryInterface(riid, ppvObj); }
STDMETHODIMP_(ULONG) ProxyIDirect3D9::AddRef() { m_ref++; return m_real->AddRef(); }
STDMETHODIMP_(ULONG) ProxyIDirect3D9::Release() { ULONG r = m_real->Release(); if(--m_ref==0) delete this; return r; }
STDMETHODIMP ProxyIDirect3D9::RegisterSoftwareDevice(void* p) { return m_real->RegisterSoftwareDevice(p); }
STDMETHODIMP_(UINT) ProxyIDirect3D9::GetAdapterCount() { return m_real->GetAdapterCount(); }
STDMETHODIMP ProxyIDirect3D9::GetAdapterIdentifier(UINT A,DWORD F,D3DADAPTER_IDENTIFIER9* p) { return m_real->GetAdapterIdentifier(A,F,p); }
STDMETHODIMP_(UINT) ProxyIDirect3D9::GetAdapterModeCount(UINT A,D3DFORMAT F) { return m_real->GetAdapterModeCount(A,F); }
STDMETHODIMP ProxyIDirect3D9::EnumAdapterModes(UINT A,D3DFORMAT F,UINT M,D3DDISPLAYMODE* p) { return m_real->EnumAdapterModes(A,F,M,p); }
STDMETHODIMP ProxyIDirect3D9::GetAdapterDisplayMode(UINT A,D3DDISPLAYMODE* p) { return m_real->GetAdapterDisplayMode(A,p); }
STDMETHODIMP ProxyIDirect3D9::CheckDeviceType(UINT A,D3DDEVTYPE T,D3DFORMAT AF,D3DFORMAT BF,BOOL W) { return m_real->CheckDeviceType(A,T,AF,BF,W); }
STDMETHODIMP ProxyIDirect3D9::CheckDeviceFormat(UINT A,D3DDEVTYPE T,D3DFORMAT AF,DWORD U,D3DRESOURCETYPE R,D3DFORMAT CF) { return m_real->CheckDeviceFormat(A,T,AF,U,R,CF); }
STDMETHODIMP ProxyIDirect3D9::CheckDeviceMultiSampleType(UINT A,D3DDEVTYPE T,D3DFORMAT SF,BOOL W,D3DMULTISAMPLE_TYPE MT,DWORD* Q) { return m_real->CheckDeviceMultiSampleType(A,T,SF,W,MT,Q); }
STDMETHODIMP ProxyIDirect3D9::CheckDepthStencilMatch(UINT A,D3DDEVTYPE T,D3DFORMAT AF,D3DFORMAT RTF,D3DFORMAT DSF) { return m_real->CheckDepthStencilMatch(A,T,AF,RTF,DSF); }
STDMETHODIMP ProxyIDirect3D9::CheckDeviceFormatConversion(UINT A,D3DDEVTYPE T,D3DFORMAT SF,D3DFORMAT TF) { return m_real->CheckDeviceFormatConversion(A,T,SF,TF); }
STDMETHODIMP ProxyIDirect3D9::GetDeviceCaps(UINT A,D3DDEVTYPE T,D3DCAPS9* p) { return m_real->GetDeviceCaps(A,T,p); }
STDMETHODIMP_(HMONITOR) ProxyIDirect3D9::GetAdapterMonitor(UINT A) { return m_real->GetAdapterMonitor(A); }
STDMETHODIMP ProxyIDirect3D9::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface) {
    IDirect3DDevice9* realDev = nullptr;
    HRESULT hr = m_real->CreateDevice(Adapter,DeviceType,hFocusWindow,BehaviorFlags,pPresentationParameters,&realDev);
    if(FAILED(hr)) return hr;
    *ppReturnedDeviceInterface = new ProxyIDirect3DDevice9(realDev, this);
    return S_OK;
}

ProxyIDirect3DDevice9::ProxyIDirect3DDevice9(IDirect3DDevice9* real, IDirect3D9* parent) : m_real(real), m_parent(parent) { parent->AddRef(); }
STDMETHODIMP ProxyIDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj) { return m_real->QueryInterface(riid, ppvObj); }
STDMETHODIMP_(ULONG) ProxyIDirect3DDevice9::AddRef() { m_ref++; return m_real->AddRef(); }
STDMETHODIMP_(ULONG) ProxyIDirect3DDevice9::Release() { 
    ULONG r = m_real->Release(); 
    if(--m_ref==0) { 
        m_parent->Release();
        delete this; 
    } 
    return r; 
}

// Forward all except Present
#define FWD(x) STDMETHODIMP ProxyIDirect3DDevice9::x { return m_real->x; }
STDMETHODIMP ProxyIDirect3DDevice9::TestCooperativeLevel() { return m_real->TestCooperativeLevel(); }
STDMETHODIMP_(UINT) ProxyIDirect3DDevice9::GetAvailableTextureMem() { return m_real->GetAvailableTextureMem(); }
STDMETHODIMP ProxyIDirect3DDevice9::EvictManagedResources() { return m_real->EvictManagedResources(); }
STDMETHODIMP ProxyIDirect3DDevice9::GetDirect3D(IDirect3D9** pp) { *pp=m_parent; m_parent->AddRef(); return S_OK; }
STDMETHODIMP ProxyIDirect3DDevice9::GetDeviceCaps(D3DCAPS9* p) { return m_real->GetDeviceCaps(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetDisplayMode(UINT a,D3DDISPLAYMODE* p) { return m_real->GetDisplayMode(a,p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* p) { return m_real->GetCreationParameters(p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetCursorProperties(UINT X,UINT Y,IDirect3DSurface9* p) { return m_real->SetCursorProperties(X,Y,p); }
STDMETHODIMP_(void) ProxyIDirect3DDevice9::SetCursorPosition(int X,int Y,DWORD F) { m_real->SetCursorPosition(X,Y,F); }
STDMETHODIMP_(BOOL) ProxyIDirect3DDevice9::ShowCursor(BOOL b) { return m_real->ShowCursor(b); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* p,IDirect3DSwapChain9** q) { return m_real->CreateAdditionalSwapChain(p,q); }
STDMETHODIMP ProxyIDirect3DDevice9::GetSwapChain(UINT a,IDirect3DSwapChain9** p) { return m_real->GetSwapChain(a,p); }
STDMETHODIMP_(UINT) ProxyIDirect3DDevice9::GetNumberOfSwapChains() { return m_real->GetNumberOfSwapChains(); }
STDMETHODIMP ProxyIDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* p) { return m_real->Reset(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetBackBuffer(UINT a,UINT b,D3DBACKBUFFER_TYPE c,IDirect3DSurface9** d) { return m_real->GetBackBuffer(a,b,c,d); }
STDMETHODIMP ProxyIDirect3DDevice9::GetRasterStatus(UINT a,D3DRASTER_STATUS* p) { return m_real->GetRasterStatus(a,p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetDialogBoxMode(BOOL b) { return m_real->SetDialogBoxMode(b); }
STDMETHODIMP_(void) ProxyIDirect3DDevice9::SetGammaRamp(UINT a,DWORD b,CONST D3DGAMMA_RAMP* c) { m_real->SetGammaRamp(a,b,c); }
STDMETHODIMP_(void) ProxyIDirect3DDevice9::GetGammaRamp(UINT a,D3DGAMMA_RAMP* p) { m_real->GetGammaRamp(a,p); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateTexture(UINT W,UINT H,UINT L,DWORD U,D3DFORMAT F,D3DPOOL P,IDirect3DTexture9** pp,HANDLE* h) { return m_real->CreateTexture(W,H,L,U,F,P,pp,h); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateVolumeTexture(UINT W,UINT H,UINT D,UINT L,DWORD U,D3DFORMAT F,D3DPOOL P,IDirect3DVolumeTexture9** pp,HANDLE* h) { return m_real->CreateVolumeTexture(W,H,D,L,U,F,P,pp,h); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateCubeTexture(UINT E,UINT L,DWORD U,D3DFORMAT F,D3DPOOL P,IDirect3DCubeTexture9** pp,HANDLE* h) { return m_real->CreateCubeTexture(E,L,U,F,P,pp,h); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateVertexBuffer(UINT L,DWORD U,DWORD FVF,D3DPOOL P,IDirect3DVertexBuffer9** pp,HANDLE* h) { return m_real->CreateVertexBuffer(L,U,FVF,P,pp,h); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateIndexBuffer(UINT L,DWORD U,D3DFORMAT F,D3DPOOL P,IDirect3DIndexBuffer9** pp,HANDLE* h) { return m_real->CreateIndexBuffer(L,U,F,P,pp,h); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateRenderTarget(UINT W,UINT H,D3DFORMAT F,D3DMULTISAMPLE_TYPE M,DWORD Q,BOOL L,IDirect3DSurface9** pp,HANDLE* h) { return m_real->CreateRenderTarget(W,H,F,M,Q,L,pp,h); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateDepthStencilSurface(UINT W,UINT H,D3DFORMAT F,D3DMULTISAMPLE_TYPE M,DWORD Q,BOOL D,IDirect3DSurface9** pp,HANDLE* h) { return m_real->CreateDepthStencilSurface(W,H,F,M,Q,D,pp,h); }
STDMETHODIMP ProxyIDirect3DDevice9::UpdateSurface(IDirect3DSurface9* a,CONST RECT* b,IDirect3DSurface9* c,CONST POINT* d) { return m_real->UpdateSurface(a,b,c,d); }
STDMETHODIMP ProxyIDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* a,IDirect3DBaseTexture9* b) { return m_real->UpdateTexture(a,b); }
STDMETHODIMP ProxyIDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* a,IDirect3DSurface9* b) { return m_real->GetRenderTargetData(a,b); }
STDMETHODIMP ProxyIDirect3DDevice9::GetFrontBufferData(UINT a,IDirect3DSurface9* b) { return m_real->GetFrontBufferData(a,b); }
STDMETHODIMP ProxyIDirect3DDevice9::StretchRect(IDirect3DSurface9* a,CONST RECT* b,IDirect3DSurface9* c,CONST RECT* d,D3DTEXTUREFILTERTYPE e) { return m_real->StretchRect(a,b,c,d,e); }
STDMETHODIMP ProxyIDirect3DDevice9::ColorFill(IDirect3DSurface9* a,CONST RECT* b,D3DCOLOR c) { return m_real->ColorFill(a,b,c); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateOffscreenPlainSurface(UINT W,UINT H,D3DFORMAT F,D3DPOOL P,IDirect3DSurface9** pp,HANDLE* h) { return m_real->CreateOffscreenPlainSurface(W,H,F,P,pp,h); }
STDMETHODIMP ProxyIDirect3DDevice9::SetRenderTarget(DWORD a,IDirect3DSurface9* b) { return m_real->SetRenderTarget(a,b); }
STDMETHODIMP ProxyIDirect3DDevice9::GetRenderTarget(DWORD a,IDirect3DSurface9** b) { return m_real->GetRenderTarget(a,b); }
STDMETHODIMP ProxyIDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* a) { return m_real->SetDepthStencilSurface(a); }
STDMETHODIMP ProxyIDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** a) { return m_real->GetDepthStencilSurface(a); }
STDMETHODIMP ProxyIDirect3DDevice9::BeginScene() { return m_real->BeginScene(); }
STDMETHODIMP ProxyIDirect3DDevice9::EndScene() { return m_real->EndScene(); }
STDMETHODIMP ProxyIDirect3DDevice9::Clear(DWORD C,CONST D3DRECT* R,DWORD F,D3DCOLOR Col,float Z,DWORD S) { return m_real->Clear(C,R,F,Col,Z,S); }
STDMETHODIMP ProxyIDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE S,CONST D3DMATRIX* M) { return m_real->SetTransform(S,M); }
STDMETHODIMP ProxyIDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE S,D3DMATRIX* M) { return m_real->GetTransform(S,M); }
STDMETHODIMP ProxyIDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE S,CONST D3DMATRIX* M) { return m_real->MultiplyTransform(S,M); }
STDMETHODIMP ProxyIDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* p) { return m_real->SetViewport(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetViewport(D3DVIEWPORT9* p) { return m_real->GetViewport(p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* p) { return m_real->SetMaterial(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetMaterial(D3DMATERIAL9* p) { return m_real->GetMaterial(p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetLight(DWORD I,CONST D3DLIGHT9* p) { return m_real->SetLight(I,p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetLight(DWORD I,D3DLIGHT9* p) { return m_real->GetLight(I,p); }
STDMETHODIMP ProxyIDirect3DDevice9::LightEnable(DWORD I,BOOL E) { return m_real->LightEnable(I,E); }
STDMETHODIMP ProxyIDirect3DDevice9::GetLightEnable(DWORD I,BOOL* p) { return m_real->GetLightEnable(I,p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetClipPlane(DWORD I,CONST float* p) { return m_real->SetClipPlane(I,p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetClipPlane(DWORD I,float* p) { return m_real->GetClipPlane(I,p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE S,DWORD V) { return m_real->SetRenderState(S,V); }
STDMETHODIMP ProxyIDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE S,DWORD* p) { return m_real->GetRenderState(S,p); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE T,IDirect3DStateBlock9** pp) { return m_real->CreateStateBlock(T,pp); }
STDMETHODIMP ProxyIDirect3DDevice9::BeginStateBlock() { return m_real->BeginStateBlock(); }
STDMETHODIMP ProxyIDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** pp) { return m_real->EndStateBlock(pp); }
STDMETHODIMP ProxyIDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* p) { return m_real->SetClipStatus(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* p) { return m_real->GetClipStatus(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetTexture(DWORD S,IDirect3DBaseTexture9** pp) { return m_real->GetTexture(S,pp); }
STDMETHODIMP ProxyIDirect3DDevice9::SetTexture(DWORD S,IDirect3DBaseTexture9* p) { return m_real->SetTexture(S,p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetTextureStageState(DWORD S,D3DTEXTURESTAGESTATETYPE T,DWORD* p) { return m_real->GetTextureStageState(S,T,p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetTextureStageState(DWORD S,D3DTEXTURESTAGESTATETYPE T,DWORD V) { return m_real->SetTextureStageState(S,T,V); }
STDMETHODIMP ProxyIDirect3DDevice9::GetSamplerState(DWORD S,D3DSAMPLERSTATETYPE T,DWORD* p) { return m_real->GetSamplerState(S,T,p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetSamplerState(DWORD S,D3DSAMPLERSTATETYPE T,DWORD V) { return m_real->SetSamplerState(S,T,V); }
STDMETHODIMP ProxyIDirect3DDevice9::ValidateDevice(DWORD* p) { return m_real->ValidateDevice(p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetPaletteEntries(UINT P,CONST PALETTEENTRY* E) { return m_real->SetPaletteEntries(P,E); }
STDMETHODIMP ProxyIDirect3DDevice9::GetPaletteEntries(UINT P,PALETTEENTRY* E) { return m_real->GetPaletteEntries(P,E); }
STDMETHODIMP ProxyIDirect3DDevice9::SetCurrentTexturePalette(UINT P) { return m_real->SetCurrentTexturePalette(P); }
STDMETHODIMP ProxyIDirect3DDevice9::GetCurrentTexturePalette(UINT* P) { return m_real->GetCurrentTexturePalette(P); }
STDMETHODIMP ProxyIDirect3DDevice9::SetScissorRect(CONST RECT* p) { return m_real->SetScissorRect(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetScissorRect(RECT* p) { return m_real->GetScissorRect(p); }
STDMETHODIMP ProxyIDirect3DDevice9::SetSoftwareVertexProcessing(BOOL b) { return m_real->SetSoftwareVertexProcessing(b); }
STDMETHODIMP_(BOOL) ProxyIDirect3DDevice9::GetSoftwareVertexProcessing() { return m_real->GetSoftwareVertexProcessing(); }
STDMETHODIMP ProxyIDirect3DDevice9::SetNPatchMode(float n) { return m_real->SetNPatchMode(n); }
STDMETHODIMP_(float) ProxyIDirect3DDevice9::GetNPatchMode() { return m_real->GetNPatchMode(); }
STDMETHODIMP ProxyIDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE T,UINT S,UINT C) { return m_real->DrawPrimitive(T,S,C); }
STDMETHODIMP ProxyIDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE T,INT B,UINT Min,UINT Num,UINT s,UINT p) { return m_real->DrawIndexedPrimitive(T,B,Min,Num,s,p); }
STDMETHODIMP ProxyIDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE T,UINT C,CONST void* D,UINT S) { return m_real->DrawPrimitiveUP(T,C,D,S); }
STDMETHODIMP ProxyIDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE T,UINT Min,UINT Num,UINT C,CONST void* I,D3DFORMAT F,CONST void* V,UINT S) { return m_real->DrawIndexedPrimitiveUP(T,Min,Num,C,I,F,V,S); }
STDMETHODIMP ProxyIDirect3DDevice9::ProcessVertices(UINT S,UINT D,UINT C,IDirect3DVertexBuffer9* B,IDirect3DVertexDeclaration9* V,DWORD F) { return m_real->ProcessVertices(S,D,C,B,V,F); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* E,IDirect3DVertexDeclaration9** pp) { return m_real->CreateVertexDeclaration(E,pp); }
STDMETHODIMP ProxyIDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* p) { return m_real->SetVertexDeclaration(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** pp) { return m_real->GetVertexDeclaration(pp); }
STDMETHODIMP ProxyIDirect3DDevice9::SetFVF(DWORD F) { return m_real->SetFVF(F); }
STDMETHODIMP ProxyIDirect3DDevice9::GetFVF(DWORD* p) { return m_real->GetFVF(p); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateVertexShader(CONST DWORD* F,IDirect3DVertexShader9** pp) { return m_real->CreateVertexShader(F,pp); }
STDMETHODIMP ProxyIDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* p) { return m_real->SetVertexShader(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** pp) { return m_real->GetVertexShader(pp); }
STDMETHODIMP ProxyIDirect3DDevice9::SetVertexShaderConstantF(UINT S,CONST float* D,UINT C) { return m_real->SetVertexShaderConstantF(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::GetVertexShaderConstantF(UINT S,float* D,UINT C) { return m_real->GetVertexShaderConstantF(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::SetVertexShaderConstantI(UINT S,CONST int* D,UINT C) { return m_real->SetVertexShaderConstantI(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::GetVertexShaderConstantI(UINT S,int* D,UINT C) { return m_real->GetVertexShaderConstantI(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::SetVertexShaderConstantB(UINT S,CONST BOOL* D,UINT C) { return m_real->SetVertexShaderConstantB(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::GetVertexShaderConstantB(UINT S,BOOL* D,UINT C) { return m_real->GetVertexShaderConstantB(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::SetStreamSource(UINT S,IDirect3DVertexBuffer9* D,UINT O,UINT St) { return m_real->SetStreamSource(S,D,O,St); }
STDMETHODIMP ProxyIDirect3DDevice9::GetStreamSource(UINT S,IDirect3DVertexBuffer9** D,UINT* O,UINT* St) { return m_real->GetStreamSource(S,D,O,St); }
STDMETHODIMP ProxyIDirect3DDevice9::SetStreamSourceFreq(UINT S,UINT F) { return m_real->SetStreamSourceFreq(S,F); }
STDMETHODIMP ProxyIDirect3DDevice9::GetStreamSourceFreq(UINT S,UINT* F) { return m_real->GetStreamSourceFreq(S,F); }
STDMETHODIMP ProxyIDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* p) { return m_real->SetIndices(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** pp) { return m_real->GetIndices(pp); }
STDMETHODIMP ProxyIDirect3DDevice9::CreatePixelShader(CONST DWORD* F,IDirect3DPixelShader9** pp) { return m_real->CreatePixelShader(F,pp); }
STDMETHODIMP ProxyIDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* p) { return m_real->SetPixelShader(p); }
STDMETHODIMP ProxyIDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** pp) { return m_real->GetPixelShader(pp); }
STDMETHODIMP ProxyIDirect3DDevice9::SetPixelShaderConstantF(UINT S,CONST float* D,UINT C) { return m_real->SetPixelShaderConstantF(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::GetPixelShaderConstantF(UINT S,float* D,UINT C) { return m_real->GetPixelShaderConstantF(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::SetPixelShaderConstantI(UINT S,CONST int* D,UINT C) { return m_real->SetPixelShaderConstantI(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::GetPixelShaderConstantI(UINT S,int* D,UINT C) { return m_real->GetPixelShaderConstantI(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::SetPixelShaderConstantB(UINT S,CONST BOOL* D,UINT C) { return m_real->SetPixelShaderConstantB(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::GetPixelShaderConstantB(UINT S,BOOL* D,UINT C) { return m_real->GetPixelShaderConstantB(S,D,C); }
STDMETHODIMP ProxyIDirect3DDevice9::DrawRectPatch(UINT H,CONST float* S,CONST D3DRECTPATCH_INFO* I) { return m_real->DrawRectPatch(H,S,I); }
STDMETHODIMP ProxyIDirect3DDevice9::DrawTriPatch(UINT H,CONST float* S,CONST D3DTRIPATCH_INFO* I) { return m_real->DrawTriPatch(H,S,I); }
STDMETHODIMP ProxyIDirect3DDevice9::DeletePatch(UINT H) { return m_real->DeletePatch(H); }
STDMETHODIMP ProxyIDirect3DDevice9::CreateQuery(D3DQUERYTYPE T,IDirect3DQuery9** pp) { return m_real->CreateQuery(T,pp); }

void ProxyIDirect3DDevice9::DrawRedSyncPixel() {
    if (!g_pWinXrApi) return;
    // FrameID from latest pose - use red channel
    int frameID = g_pWinXrApi->GetLastRenderedFrameID() & 0xFF;
    
    // Draw 10x10 red block top-left - WinlatorXR scrapes this pixel to know which frame was presented
    // Use state block so game state restored
    IDirect3DStateBlock9* sb = nullptr;
    m_real->CreateStateBlock(D3DSBT_ALL, &sb);
    
    m_real->SetRenderState(D3DRS_ZENABLE, FALSE);
    m_real->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    m_real->SetRenderState(D3DRS_FOGENABLE, FALSE);
    m_real->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    
    struct Vertex { float x,y,z,rhw; DWORD color; };
    Vertex verts[6] = {
        {0,0,0,1, D3DCOLOR_ARGB(255, frameID, 0, 0)},
        {10,0,0,1, D3DCOLOR_ARGB(255, frameID, 0, 0)},
        {0,10,0,1, D3DCOLOR_ARGB(255, frameID, 0, 0)},
        {0,10,0,1, D3DCOLOR_ARGB(255, frameID, 0, 0)},
        {10,0,0,1, D3DCOLOR_ARGB(255, frameID, 0, 0)},
        {10,10,0,1, D3DCOLOR_ARGB(255, frameID, 0, 0)},
    };
    
    m_real->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, verts, sizeof(Vertex));
    
    if (sb) {
        sb->Apply();
        sb->Release();
    }
}

STDMETHODIMP ProxyIDirect3DDevice9::Present(CONST RECT* src,CONST RECT* dest,HWND win,CONST RGNDATA* dirty) {
    // Update last rendered FrameID from latest packet
    if (g_pWinXrApi) {
        XrPacket pkt;
        if (g_pWinXrApi->GetPose(pkt)) {
            g_pWinXrApi->SetLastRenderedFrameID(pkt.FrameID);
            // Send haptics reply: mono VR screen mode = MODE_VR=2, MODE_3D=0
            g_pWinXrApi->SendHaptics(0,0,2,0,90.f,90.f);
        }
    }
    
    DrawRedSyncPixel();
    
    return m_real->Present(src,dest,win,dirty);
}

// Exported Direct3DCreate9 - entry point for proxy
extern "C" IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {
    if (!g_pfnRealDirect3DCreate9) {
        char sysPath[MAX_PATH];
        GetSystemDirectoryA(sysPath, MAX_PATH);
        strcat_s(sysPath, "\\d3d9.dll");
        g_hRealD3D9 = LoadLibraryA(sysPath);
        if (!g_hRealD3D9) return nullptr;
        g_pfnRealDirect3DCreate9 = (PFN_Direct3DCreate9)GetProcAddress(g_hRealD3D9, "Direct3DCreate9");
    }
    IDirect3D9* real = g_pfnRealDirect3DCreate9(SDKVersion);
    if (!real) return nullptr;
    return new ProxyIDirect3D9(real);
}
