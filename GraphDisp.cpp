
#include "GraphDisp.h"
#include "MKGenException.h"
#include "system.h"

#if defined(WINDOWS)
#include "wtypes.h"
#endif

namespace MKBasic {

/*
 *--------------------------------------------------------------------
 * Method:		GraphDisp()
 * Purpose:		Class default constructor.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
GraphDisp::GraphDisp()
{
	Initialize();
}

/*
 *--------------------------------------------------------------------
 * Method:		GraphDisp()
 * Purpose:		Class custom constructor.
 * Arguments:	width, height - integer, graphical display dimensions
 *														(virtual dimensions as opposed to
 *														 to actual dimensions GRAPHDISP_MAXW,
 *														 GRAPHDISP_MAXH)
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
GraphDisp::GraphDisp(int width, int height)
{
	mWidth = width;
	mHeight = height;
	Initialize();
}

/*
 *--------------------------------------------------------------------
 * Method:		~GraphDisp()
 * Purpose:		Class destructor.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
GraphDisp::~GraphDisp()
{
	mContLoop = false;
	while (mMainLoopActive);
	SDL_DestroyRenderer(mpRenderer);
	SDL_DestroyWindow(mpWindow);
	SDL_Quit();
}

/*
 *--------------------------------------------------------------------
 * Method:		Initialize()
 * Purpose:		Initialize class members, objects.
 * Arguments:	n/a
 * Returns:		n/a
 * Problems:
 *    SDL_CreateWindow with flags:
 *    SDL_WINDOW_SHOWN|SDL_WINDOW_BORDERLESS|SDL_WINDOW_RESIZABLE
 *    creates window with no title/icon/system buttons, but with
 *    a frame, which can be used to resize it. The problem is, when
 *    it is resized, it stops updating correctly.
 *--------------------------------------------------------------------
 */
void GraphDisp::Initialize()
{
	int desk_w, desk_h, winbd_top = 5, winbd_right = 5;

	GetDesktopResolution(desk_w, desk_h);
	// Available in version > 2.0.4
	//SDL_GetWindowBordersSize(mpWindow, &winbd_top, NULL, NULL, &winbd_right);
	mWinPosX = desk_w - GRAPHDISP_MAXW - winbd_right;
	mWinPosY = winbd_top;

	SDL_Init(SDL_INIT_VIDEO);

	mpWindow = SDL_CreateWindow(
		"GraphDisp", 
		mWinPosX,
		mWinPosY,
		GRAPHDISP_MAXW, 
		GRAPHDISP_MAXH, 
		SDL_WINDOW_SHOWN|SDL_WINDOW_BORDERLESS|SDL_WINDOW_RESIZABLE
		);

	if (NULL == mpWindow) {
		throw MKGenException(SDL_GetError());
	}

  // Get window surface
  mpSurface = SDL_GetWindowSurface(mpWindow);

	// Create renderer for window
	mpRenderer = SDL_CreateRenderer(mpWindow, -1, SDL_RENDERER_SOFTWARE);
  //mpRenderer = SDL_GetRenderer(mpWindow);	

	if (NULL == mpRenderer) {
		throw MKGenException(SDL_GetError());
	}	  

  Clear();

}

/*
 *--------------------------------------------------------------------
 * Method:		ClearScreen()
 * Purpose:		Clear the surface. Update screen.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::ClearScreen()
{
	Clear();
  UpdateSurface();	
}

/*
 *--------------------------------------------------------------------
 * Method:		Clear()
 * Purpose:		Clear the surface.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::Clear()
{
  //Fill the surface with background color
  SDL_FillRect(mpSurface, NULL, SDL_MapRGB(mpSurface->format, 
  																				 mBgRgbR, 
  																				 mBgRgbG, 
  																				 mBgRgbB));
}

/*
 *--------------------------------------------------------------------
 * Method:		UpdateSurface()
 * Purpose:		Update window surface.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::UpdateSurface()
{
  //Update the surface
  SDL_UpdateWindowSurface(mpWindow);
}

/*
 *--------------------------------------------------------------------
 * Method:		Update()
 * Purpose:		Update window surface (public).
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::Update()
{
  //Update the surface
  UpdateSurface();
}

/*
 *--------------------------------------------------------------------
 * Method:		RenderPixel()
 * Purpose:		Set or unset pixel (scaled) on graphics display
 * 						surface.
 * Arguments:	x, y - integer, virtual pixel coordinates
 *						set - boolean, set or unset pixel
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::RenderPixel(int x, int y, bool set)
{
	SDL_Rect rtf;

	rtf.x = x * mPixelSizeX; rtf.y = y * mPixelSizeY;
	rtf.w = mPixelSizeX;
	rtf.h = mPixelSizeY;

	int rgb_r = 0, rgb_g = 0, rgb_b = 0;

	if (set) {
		rgb_r = mFgRgbR;
		rgb_g = mFgRgbG;
		rgb_b = mFgRgbB;
	} else {
		rgb_r = mBgRgbR;
		rgb_g = mBgRgbG;
		rgb_b = mBgRgbB;
	}

	// set or unset pixel
	SDL_FillRect(mpSurface, &rtf, SDL_MapRGB(mpSurface->format, rgb_r, rgb_g, rgb_b));

  //Update the surface
  SDL_UpdateWindowSurface(mpWindow);	
}

/*
 *--------------------------------------------------------------------
 * Method:		SetPixel()
 * Purpose:		Set pixel (scaled) on graphics display surface.
 * Arguments: x, y - pixel coordinates
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::SetPixel(int x, int y)
{
	RenderPixel(x, y, true);
}

/*
 *--------------------------------------------------------------------
 * Method:		ErasePixel()
 * Purpose:		Unset (erase) pixel (paint with BG color) on graphics
 *            display surface.
 * Arguments: x, y - pixel coordinates
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::ErasePixel(int x, int y)
{
	RenderPixel(x, y, false);
}

/*
 *--------------------------------------------------------------------
 * Method:		DrawLine()
 * Purpose:		Draw or erase line between specified points.
 * Arguments:	x1, y1 - coordinates of first point
 *            x2, y2 - coordinates of second point
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::DrawLine(int x1, int y1, int x2, int y2, bool draworerase)
{
	int colR = mFgRgbR, colG = mFgRgbG, colB = mFgRgbB;

	if (false == draworerase) {
		colR = mBgRgbR;
		colG = mBgRgbG;
		colB = mBgRgbB;
	}

	SDL_SetRenderDrawColor(mpRenderer, colR, colG, colB, SDL_ALPHA_OPAQUE);
	SDL_RenderSetLogicalSize(mpRenderer, mWidth, mHeight);
	SDL_RenderSetScale(mpRenderer, mPixelSizeX, mPixelSizeY);
	SDL_RenderDrawLine(mpRenderer, x1, y1, x2, y2);
	SDL_RenderPresent(mpRenderer);
}

/*
 *--------------------------------------------------------------------
 * Method:		DrawLine()
 * Purpose:		Draw line between specified points.
 * Arguments:	x1, y1 - coordinates of first point
 *            x2, y2 - coordinates of second point
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::DrawLine(int x1, int y1, int x2, int y2)
{
	DrawLine(x1, y1, x2, y2, true);
}

/*
 *--------------------------------------------------------------------
 * Method:		EraseLine()
 * Purpose:		Erase line between specified points (draw with BG color)
 * Arguments:	x1, y1 - coordinates of first point
 *            x2, y2 - coordinates of second point
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::EraseLine(int x1, int y1, int x2, int y2)
{
	DrawLine(x1, y1, x2, y2, false);
}

/*
 *--------------------------------------------------------------------
 * Method:		SetBgColor()
 * Purpose:		Set background color.
 * Arguments:	r, g, b - integer, red, green and blue intensities
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::SetBgColor(int r, int g, int b)
{
	mBgRgbR = r;
	mBgRgbG = g;
	mBgRgbB = b;
}

/*
 *--------------------------------------------------------------------
 * Method:		SetFgColor()
 * Purpose:		Set foreground color.
 * Arguments:	r, g, b - integer, red, green and blue intensities
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::SetFgColor(int r, int g, int b)
{
	mFgRgbR = r;
	mFgRgbG = g;
	mFgRgbB = b;
}

/*
 *--------------------------------------------------------------------
 * Method:		MainLoop()
 * Purpose:		The main loop to process SDL events and update window.
 *            This is a global function meant to run in a thread.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void MainLoop(GraphDisp *pgd)
{
	pgd->mMainLoopActive = true;
	while (pgd->mContLoop) {
		pgd->ReadEvents();
		pgd->Update();
  }
  pgd->mMainLoopActive = false;
}

/*
 *--------------------------------------------------------------------
 * Method:		Start()
 * Purpose:		Starts MainLoop in a thread.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::Start(GraphDisp *pgd)
{
	mMainLoopThread = thread(MainLoop,pgd);
}

/*
 *--------------------------------------------------------------------
 * Method:		Stop()
 * Purpose:		Stop MainLoop thread.
 * Arguments:	n/a
 * Returns:		n/a
 *--------------------------------------------------------------------
 */
void GraphDisp::Stop()
{
	mContLoop = false;
	mMainLoopThread.join();
}

/*
 *--------------------------------------------------------------------
 * Method:		
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
bool GraphDisp::IsMainLoopActive()
{
	return mMainLoopActive;
}

/*
 *--------------------------------------------------------------------
 * Method:		ReadEvents()
 * Purpose:		Read events and perform actions when needed.	
 * Arguments:	n/a
 * Returns:		n/a
 * Problems:
 *    By blitting (copying) surface I wanted to avoid loosing already
 *    drawn pixels during windows resize, but this doesn't work.
 *--------------------------------------------------------------------
 */
void GraphDisp::ReadEvents()
{
		SDL_Event e;
		SDL_Surface *pTmpSurface;

		while (SDL_PollEvent(&e)) {

			switch (e.type) {

				case SDL_QUIT:
					mContLoop = false;
					break;

				case SDL_WINDOWEVENT:
					if (SDL_WINDOWEVENT_RESIZED == e.window.event) {

						pTmpSurface = SDL_CreateRGBSurface(0, 
																							 mpSurface->w,
																							 mpSurface->h,
																							 mpSurface->format->BitsPerPixel,
                                  						 mpSurface->format->Rmask, 
                                  						 mpSurface->format->Gmask,
                                  						 mpSurface->format->Bmask, 
                                  						 mpSurface->format->Amask);
						SDL_SetWindowSize(mpWindow, GRAPHDISP_MAXW, GRAPHDISP_MAXH);
						mpSurface = SDL_GetWindowSurface(mpWindow);
						SDL_SetWindowPosition(mpWindow, mWinPosX, mWinPosY);
						SDL_SetSurfaceAlphaMod(pTmpSurface, 0);
						SDL_BlitSurface(pTmpSurface, 0, mpSurface, 0);						
						UpdateSurface();
						SDL_FreeSurface(pTmpSurface);						

					} else if (SDL_WINDOWEVENT_FOCUS_GAINED == e.window.event) {

						SDL_RaiseWindow(mpWindow);
						
					}
					break;

				default:
					break;
			}

		}
}

/*
 *--------------------------------------------------------------------
 * Method:		
 * Purpose:
 * Arguments:
 * Returns:
 *--------------------------------------------------------------------
 */
void GraphDisp::GetDesktopResolution(int& horizontal, int& vertical)
{
#if defined(WINDOWS)	
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right;
   vertical = desktop.bottom;
#else
   horizontal = GRAPHDISP_MAXW;
   vertical = GRAPHDISP_MAXH;
#endif   
}

} // namespace MKBasic