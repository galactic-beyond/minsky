/*
  @copyright Steve Keen 2021
  @author Janak Porwal
  This file is part of Minsky.

  Minsky is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minsky is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
*/
#define NTDDI_VERSION NTDDI_WINBLUE
#include "windowInformation.h"
#include "minsky_epilogue.h"

#include <stdexcept>
#include <string>

#if defined(CAIRO_HAS_WIN32_SURFACE) && !defined(__CYGWIN__)
#define USE_WIN32_SURFACE
#elif defined(CAIRO_HAS_XLIB_SURFACE) && !defined(MAC_OSX_TK)
#define USE_X11
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#endif


#ifdef _WIN32
#undef Realloc
#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <winuser.h>
#include <shellscalingapi.h>
#ifdef USE_WIN32_SURFACE
#include <cairo/cairo-win32.h>
#endif
#endif

#if defined(MAC_OSX_TK)
#include <Carbon/Carbon.h>
#include <cairo/cairo-quartz.h>
#include "getContext.h"
#endif

using namespace std;
using namespace ecolab;

//#define MINSKY_CANVAS_BACKGROUND_COLOR 0xee5a5add
//#define MINSKY_CANVAS_BACKGROUND_COLOR_OTHER 0xee5add5a

#define MINSKY_CANVAS_BACKGROUND_COLOR 0x00ffffff

// TODO:: Child surface should be transparent --- or -- there has to be provision to set background color as FE has that option

namespace minsky
{

#ifdef USE_WIN32_SURFACE
#elif defined(MAC_OSX_TK)
#else
  int throwOnXError(Display *, XErrorEvent *ev)
  {
    char errorMessage[256];
    XGetErrorText(ev->display, ev->error_code, errorMessage, sizeof(errorMessage));
    throw runtime_error(errorMessage);
  }
#endif

  WindowInformation::~WindowInformation()
  {
    //cout << "~WindowInformation()"<<endl;
    bufferSurface.reset();
#ifdef USE_WIN32_SURFACE
    SelectObject(hdcMem, hOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    DestroyWindow(childWindowId);
#elif defined(MAC_OSX_TK)
#elif defined(USE_X11)
    cout << "resetting eventThread"<<endl;
    eventThread.reset(); //shut thread down before destroying window
    cout << "XFreeGC()"<<endl;
    XFreeGC(display, graphicsContext);
    cout << "XDestroyWindow(display, childWindowId)"<<endl;
    XDestroyWindow(display, childWindowId);
    cout << "XDestroyWindow(display, bufferWindowId)"<<endl;
    XDestroyWindow(display, bufferWindowId);
#endif
    //cout << "leaving ~WindowInformation()"<<endl;
  }

  const ecolab::cairo::SurfacePtr& WindowInformation::getBufferSurface()
  {
    return bufferSurface;
  }

  void WindowInformation::copyBufferToMain()
  {
    cairo_surface_flush(bufferSurface->surface());
#ifdef USE_WIN32_SURFACE
    InvalidateRect(childWindowId,nullptr,true);
    PostMessageA(childWindowId,WM_PAINT,0,0);
#elif defined(USE_X11)
    blit(0,0,childWidth,childHeight);
#endif
  }

  void WindowInformation::setRenderingFlag(bool value)
  {
    isRendering = value;
  }

  bool WindowInformation::getRenderingFlag()
  {
    return isRendering;
  }

#ifdef USE_WIN32_SURFACE
  LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
  {
    WindowInformation* winfo=reinterpret_cast<WindowInformation*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (msg)
      {
      case WM_PAINT:
        {
          RECT r;
          if (GetUpdateRect(hwnd,&r,false))
            winfo->blit(r.left, r.top, r.right-r.left, r.bottom-r.top);
        }
        return 0;
      case WM_NCHITTEST:
        return HTTRANSPARENT;
      default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
      }
  }
#endif

  void WindowInformation::blit(int x, int y, int width, int height)
  {
#ifdef USE_WIN32_SURFACE
    PAINTSTRUCT ps;
    HDC dc=BeginPaint(childWindowId, &ps);
    BitBlt(dc, x, y, width,height,hdcMem,x,y,SRCCOPY);
    EndPaint(childWindowId, &ps);
#elif defined(USE_X11)
    cout << "copy area" << endl;
    XCopyArea(display, bufferWindowId, childWindowId, graphicsContext, x, y, width, height, x, y);
    cout << "flush" << endl;
    XFlush(display);
    cout << "raise" << endl;
    XRaiseWindow(display, childWindowId);
    cout << "finished blit"<<endl;
#endif
  }

#if defined(USE_X11)
  void WindowInformation::EventThread::run()
  {
    while (running)
      try
      {
        XEvent event;
        if (winfo.getRenderingFlag() || winfo.childWindowId==-1 || !XCheckWindowEvent(winfo.display, winfo.childWindowId, ExposureMask|StructureNotifyMask, &event))
          {
            this_thread::sleep_for(50ms); //thottle, to avoid starving other threads
            continue;
          }
        cout << "event "<<event.type<<" received"<<endl;
        switch (event.type)
          {
          case Expose:
            cout << "Expose"<<endl;
            winfo.blit(event.xexpose.x, event.xexpose.y, event.xexpose.width, event.xexpose.height);
            break;
          case DestroyNotify:
            cout << "Destroy"<<endl;
            return; // exit thread, window has gone away
          }
      }
      catch (const std::exception& ex)
        {
          // absorb and log, not much else we can do with X11 errors at this point
          cerr << ex.what() << endl;
        }
  }
#endif
  
  WindowInformation::WindowInformation(uint64_t parentWin, int left, int top, int cWidth, int cHeight,
                                       const std::function<void(void)>& draw)
#ifdef MAC_OSX_TK
    : nsContext(reinterpret_cast<void*>(parentWin),left,top,cWidth,cHeight,*this), draw(draw)
#endif
  {
    //cout << "WindowInformation()"<<endl;
    offsetLeft = left;
    offsetTop = top;

    childWidth = cWidth;
    childHeight = cHeight;

#ifdef USE_WIN32_SURFACE
    parentWindowId = reinterpret_cast<HWND>(parentWin);

    // adjust everything by the monitor scale factor
    DEVICE_SCALE_FACTOR scaleFactor;
    GetScaleFactorForMonitor(MonitorFromWindow(parentWindowId, MONITOR_DEFAULTTONEAREST), &scaleFactor);
    double sf=scaleFactor/100.0;
    top*=sf;
    left*=sf;
    childWidth*=sf;
    childHeight*=sf;
    
    auto style=GetWindowLong(parentWindowId, GWL_STYLE);
    SetWindowLongPtrA(parentWindowId, GWL_STYLE, style|WS_CLIPCHILDREN);
    childWindowId=CreateWindowA("Button", "", WS_CHILD | WS_VISIBLE|WS_CLIPSIBLINGS, left, top, childWidth, childHeight, parentWindowId, nullptr, nullptr, nullptr);
    SetWindowRgn(childWindowId,CreateRectRgn(0,0,childWidth, childHeight),true);
    HDC hdc=GetDC(childWindowId);
    hdcMem=CreateCompatibleDC(hdc);
    hbmMem=CreateCompatibleBitmap(hdc, childWidth, childHeight);
    ReleaseDC(parentWindowId, hdc);
    hOld=SelectObject(hdcMem, hbmMem);
    bufferSurface.reset(new cairo::Surface(cairo_win32_surface_create(hdcMem),childWidth, childHeight));
    if (scaleFactor>0)
      cairo_surface_set_device_scale(bufferSurface->surface(), sf, sf);
    SetWindowLongPtrA(childWindowId, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    SetWindowLongPtrA(childWindowId, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(windowProc));
#elif defined(MAC_OSX_TK)
#elif defined(USE_X11)
    parentWindowId = parentWin;
    static bool errorHandlingSet = (XSetErrorHandler(throwOnXError), true);
    display = XOpenDisplay(nullptr);
    int err = XGetWindowAttributes(display, parentWin, &wAttr);
    if (err > 1)
      throw runtime_error("Invalid window: " + to_string(parentWin));

    // TODO:: Do some sanity checks on dimensions
 
    childWindowId = XCreateSimpleWindow(display, parentWin, offsetLeft, offsetTop, childWidth, childHeight, 0, 0, MINSKY_CANVAS_BACKGROUND_COLOR);
    bufferWindowId = XCreatePixmap(display, parentWin, childWidth, childHeight, wAttr.depth);
    graphicsContext=XCreateGC(display, childWindowId, 0, nullptr);
    
    XMapWindow(display, childWindowId);
    bufferSurface.reset(new cairo::Surface(cairo_xlib_surface_create(display, bufferWindowId, wAttr.visual, childWidth, childHeight), childWidth, childHeight));

    // listen to expose events
    XSelectInput(display, childWindowId, ExposureMask);
    eventThread.reset(new EventThread(*this)); // delay construction of this until after the window is created

#endif
    //cout << "leaving WindowInformation()"<<endl;
  }

  void WindowInformation::requestRedraw()
  {
#ifdef MAC_OSX_TK
    nsContext.requestRedraw();
#endif
  }

} // namespace minsky
