#include "main_shared.h"
#include "imgui.h"
#include "bgfx/bgfx.h"
#include "imgui_impl_bgfx.h"
#include "UIMain.h"

#ifdef WIN32
bool ImGui_ImplWin32_Init(void* hwnd);
void ImGui_ImplWin32_NewFrame();
void ImGui_ImplWin32_Shutdown();
#endif

void PlaformInit(void* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    bgfx::Init init;
    //init.type = commandLineParameters.mRenderAPI;
    //bgfxCallback callback;
    //init.callback = &callback;
    init.platformData.nwh = window;
    bgfx::init(init);


    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

    bgfx::frame();

    // Setup Platform/Renderer bindings
#ifdef WIN32
    ImGui_ImplWin32_Init(window);
#endif
    ImGui_Implbgfx_Init();
}

void PlatformFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    bgfx::touch(0);
    // Start the Dear ImGui frame
    ImGui_Implbgfx_NewFrame();
#ifdef WIN32
    ImGui_ImplWin32_NewFrame();
#endif
    ImGui::NewFrame();

    /*ImGui::Begin("Another Window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::End();*/
    UIMain();


    // Rendering
    ImGui::EndFrame();

    ImGui::Render();
    ImGui_Implbgfx_RenderDrawData(0, ImGui::GetDrawData());
    bgfx::frame();

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void PlatformFinalize()
{
    ImGui_Implbgfx_Shutdown();
#ifdef WIN32
    ImGui_ImplWin32_Shutdown();
#endif
    ImGui::DestroyContext();

    // Shutdown bgfx.
    bgfx::shutdown();
}