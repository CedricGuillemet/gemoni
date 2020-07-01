#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#include <X11/Xlib.h> // will include X11 which #defines None... Don't mess with order of includes.
#include <X11/Xutil.h>
#include <unistd.h> // syscall
#undef None
#include <filesystem>

#include "imgui.h"
#include "imgui_impl_bgfx.h"
#include "UIMain.h"

static const char* s_applicationName  = "Genomi";
static const char* s_applicationClass = "Genomi";

void Init(Window window)
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
    //ImGui_ImplWin32_Init(hwnd);
    ImGui_Implbgfx_Init();
}

void Frame()
{
    bgfx::touch(0);
    // Start the Dear ImGui frame
    ImGui_Implbgfx_NewFrame();
    ImGui_ImplWin32_NewFrame();
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

void Finalize()
{
    ImGui_Implbgfx_Shutdown();
    //ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // Shutdown bgfx.
    bgfx::shutdown();
}

int main(int _argc, const char* const* _argv)
{
    XInitThreads();
    Display* display = XOpenDisplay(NULL);

    int32_t screen = DefaultScreen(display);
    int32_t depth  = DefaultDepth(display, screen);
    Visual* visual = DefaultVisual(display, screen);
    Window root   = RootWindow(display, screen);
    const int width = 1024;
    const int height = 768;

    XSetWindowAttributes windowAttrs;
    windowAttrs.background_pixel = 0;
    windowAttrs.background_pixmap = 0;
    windowAttrs.border_pixel = 0;
    windowAttrs.event_mask = 0
            | ButtonPressMask
            | ButtonReleaseMask
            | ExposureMask
            | KeyPressMask
            | KeyReleaseMask
            | PointerMotionMask
            | StructureNotifyMask;

    Window window = XCreateWindow(display
                            , root
                            , 0, 0
                            , width, height, 0
                            , depth
                            , InputOutput
                            , visual
                            , CWBorderPixel|CWEventMask
                            , &windowAttrs
                            );

    // Clear window to black.
    XSetWindowAttributes attr;
    memset(&attr, 0, sizeof(attr) );
    XChangeWindowAttributes(display, window, CWBackPixel, &attr);

    const char* wmDeleteWindowName = "WM_DELETE_WINDOW";
    Atom wmDeleteWindow;
    XInternAtoms(display, (char **)&wmDeleteWindowName, 1, False, &wmDeleteWindow);
    XSetWMProtocols(display, window, &wmDeleteWindow, 1);

    XMapWindow(display, window);
    XStoreName(display, window, s_applicationName);

    XClassHint* hint = XAllocClassHint();
    hint->res_name  = const_cast<char*>(s_applicationName);
    hint->res_class = const_cast<char*>(s_applicationClass);
    XSetClassHint(display, window, hint);
    XFree(hint);

    XIM im = XOpenIM(display, NULL, NULL, NULL);

    XIC ic = XCreateIC(im
            , XNInputStyle
            , 0
            | XIMPreeditNothing
            | XIMStatusNothing
            , XNClientWindow
            , window
            , NULL
            );

    Init(window);
    
    bool exit{};
    while (!exit)
    {
        XEvent event;
        XNextEvent(display, &event);
        switch (event.type)
        {
            case Expose:
                break;
            case ClientMessage:
                if ( (Atom)event.xclient.data.l[0] == wmDeleteWindow)
                {
                    exit = true;
                }
                break;
            case ConfigureNotify:
                {
                    const XConfigureEvent& xev = event.xconfigure;
                    UpdateWindowSize(xev.width, xev.height);
                }
                break;
            case ButtonPress:
                break;
            case ButtonRelease:

                break;
            case MotionNotify:
                {
                    const XMotionEvent& xmotion = event.xmotion;
                }
                break;
        }
        Frame();
    }
    
    Finalize();
    
    XDestroyIC(ic);
    XCloseIM(im);

    XUnmapWindow(display, window);
    XDestroyWindow(display, window);
    return 0;
}
