#include <Windows.h>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_6.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

int main()
{
  CComPtr <IDXGIFactory>                         pFactory;
  CreateDXGIFactory (IID_IDXGIFactory, (void **)&pFactory.p);

  // ... wtf?
  if (! pFactory)
    return 0;

  int                    AdapterIdx = 0;
  CComPtr <IDXGIAdapter> pAdapter;

  while (SUCCEEDED (pFactory->EnumAdapters (AdapterIdx++, &pAdapter.p)))
  {
    int                   OutputIdx = 0;
    CComPtr <IDXGIOutput> pOutput;

    while (SUCCEEDED (pAdapter->EnumOutputs (OutputIdx++, &pOutput.p)))
    {
      CComQIPtr <IDXGIOutput6>
          pOutput6 ( pOutput );
      if (pOutput6 != nullptr)
      {
        DXGI_OUTPUT_DESC1    outDesc1 = { };
        pOutput6->GetDesc1 (&outDesc1);

        UINT Width  = outDesc1.DesktopCoordinates.right  -
                      outDesc1.DesktopCoordinates.left;
        UINT Height = outDesc1.DesktopCoordinates.bottom -
                      outDesc1.DesktopCoordinates.top;

        if (outDesc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
        {
          HWND hWnd =
            CreateWindow (
              L"static", L"HDR10", WS_VISIBLE | WS_POPUP        |
                  WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN |
                  WS_CLIPSIBLINGS, outDesc1.DesktopCoordinates.left,
                                   outDesc1.DesktopCoordinates.top,
                                     Width, Height,
                                      0, 0, 0, 0
            );

          DXGI_SWAP_CHAIN_DESC
            swapDesc                   = { };
            swapDesc.BufferDesc.Width  = Width;
            swapDesc.BufferDesc.Height = Height;
            swapDesc.BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
            swapDesc.BufferCount       = 3;
            swapDesc.Windowed          = TRUE;
            swapDesc.OutputWindow      = hWnd;
            swapDesc.SampleDesc        = { 1, 0 };
            swapDesc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            swapDesc.BufferUsage       = DXGI_USAGE_BACK_BUFFER;

          CComPtr <IDXGISwapChain> pSwapChain;

          if ( SUCCEEDED (
                 D3D11CreateDeviceAndSwapChain (
                    nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0x0, nullptr,
                      0, D3D11_SDK_VERSION, &swapDesc, &pSwapChain.p,
                        nullptr, nullptr, nullptr ) )
             )
          {
            CComQIPtr <IDXGISwapChain4>
                pSwapChain4 (pSwapChain);
            if (pSwapChain4 != nullptr)
            {
              DXGI_HDR_METADATA_HDR10
                metadata                           = { };
                metadata.MinMasteringLuminance     =  0;
                metadata.MaxMasteringLuminance     =
                  static_cast <UINT>   (outDesc1.MaxLuminance);
                metadata.MaxFrameAverageLightLevel =
                  static_cast <UINT16> (outDesc1.MaxLuminance);
                metadata.MaxContentLightLevel      =
                  static_cast <UINT16> (outDesc1.MaxLuminance);
                metadata.WhitePoint   [0]          =
                  static_cast <UINT16> (outDesc1.WhitePoint   [0] * 50000.0F);
                metadata.WhitePoint   [1]          =
                  static_cast <UINT16> (outDesc1.WhitePoint   [1] * 50000.0F);
                metadata.BluePrimary  [0]          =
                  static_cast <UINT16> (outDesc1.BluePrimary  [0] * 50000.0F);
                metadata.BluePrimary  [1]          =
                  static_cast <UINT16> (outDesc1.BluePrimary  [1] * 50000.0F);
                metadata.RedPrimary   [0]          =
                  static_cast <UINT16> (outDesc1.RedPrimary   [0] * 50000.0F);
                metadata.RedPrimary   [1]          =
                  static_cast <UINT16> (outDesc1.RedPrimary   [1] * 50000.0F);
                metadata.GreenPrimary [0]          =
                  static_cast <UINT16> (outDesc1.GreenPrimary [0] * 50000.0F);
                metadata.GreenPrimary [1]          =
                  static_cast <UINT16> (outDesc1.GreenPrimary [1] * 50000.0F);

              pSwapChain4->SetColorSpace1 (
                DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
              );

              if ( SUCCEEDED (
                     pSwapChain4->SetHDRMetaData (
                       DXGI_HDR_METADATA_TYPE_HDR10, sizeof (metadata),
                                                            &metadata ) )
                 )
              {
                printf ("Sanitized Display: %ws\n", outDesc1.DeviceName);
                printf (" MaxCLL=%f nits\n\n",      outDesc1.MaxLuminance);

                pSwapChain4->Present (1, 0);
                               Sleep ( 250);
                pSwapChain4->Present (1, 0);
              }
            }
          }

          DestroyWindow (hWnd);
        }
      }

      pOutput = nullptr;
    }

    pAdapter = nullptr;
  }
}
