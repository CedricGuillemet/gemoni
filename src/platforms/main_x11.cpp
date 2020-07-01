#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#include <X11/Xlib.h> // will include X11 which #defines None... Don't mess with order of includes.
#include <X11/Xutil.h>
#include <unistd.h> // syscall
#undef None
#include <filesystem>
#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_bgfx.h"
#include "UIMain.h"
#include "main_shared.h"

static const char* s_applicationName  = "Genomi";
static const char* s_applicationClass = "Genomi";

Display* display;
Window window ;

bool ImGui_ImplX11_Init(void* window)
{
    ImGuiIO& io = ImGui::GetIO();
    memset(io.KeyMap, 0, sizeof(io.KeyMap));
    /*
    io.KeyMap[ImGuiKey_Tab] = XK_Tab;
    io.KeyMap[ImGuiKey_LeftArrow] = XK_Left;
    io.KeyMap[ImGuiKey_RightArrow] = XK_Right;
    io.KeyMap[ImGuiKey_UpArrow] = XK_Up;
    io.KeyMap[ImGuiKey_DownArrow] = XK_Down;
    io.KeyMap[ImGuiKey_PageUp] = XK_Page_Up;
    io.KeyMap[ImGuiKey_PageDown] = XK_Page_Down;
    io.KeyMap[ImGuiKey_Home] = XK_Home;
    io.KeyMap[ImGuiKey_End] = XK_End;
    io.KeyMap[ImGuiKey_Insert] = XK_Insert;
    io.KeyMap[ImGuiKey_Delete] = XK_Delete;
    io.KeyMap[ImGuiKey_Backspace] = XK_BackSpace;
    io.KeyMap[ImGuiKey_Space] = XK_space;
    io.KeyMap[ImGuiKey_Enter] = XK_Return;
    io.KeyMap[ImGuiKey_Escape] = XK_Escape;
    io.KeyMap[ImGuiKey_KeyPadEnter] = XK_Return;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';
    */
    return true;
}

void ImGui_ImplX11_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Setup display size (every frame to accommodate for window resizing)
    Window root_return;
    int x_return, y_return;
    unsigned int width_return, height_return, border_width_return, depth_return;
    XGetGeometry(display, window, &root_return, &x_return, &y_return, &width_return, &height_return, &border_width_return, &depth_return);
    io.DisplaySize = ImVec2((float)(width_return), (float)(height_return));
}

void ImGui_ImplX11_Shutdown()
{

}

int main(int _argc, const char* const* _argv)
{
    XInitThreads();
    display = XOpenDisplay(NULL);

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

    window = XCreateWindow(display
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

    PlatformInit((void*)window);
    
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
        PlatformFrame();
    }
    
    PlatformFinalize();
    
    XDestroyIC(ic);
    XCloseIM(im);

    XUnmapWindow(display, window);
    XDestroyWindow(display, window);
    return 0;
}
