//******************************************************************************
// Main
// ----
//   All the graphics code is shamelessly copy/pasted from the windows documentation:
//   http://msdn.microsoft.com/en-us/library/vstudio/bb384843.aspx
//
// @author Doug Frazer
// May 2013
//******************************************************************************

#include "heatmap.h"

#include <time.h>

const char g_szClassName[] = "myWindowClass";
PAINTSTRUCT ps;
HDC hdc;
HEAT_MAP HeatMap(1);

// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
		{
			hdc = BeginPaint(hwnd, &ps);

			RECT Rect;
			GetClientRect( hwnd, &Rect );
			HRGN bgRgn = CreateRectRgnIndirect(&Rect);
			HBRUSH hBrush = CreateSolidBrush(RGB(200,200,200));
			FillRgn(hdc, bgRgn, hBrush);
			HeatMap.DrawBitmap(hdc, GOLD_DROP, 0, 0, 1000, 1000);

			EndPaint(hwnd, &ps);
			break;
		}
	case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
        break;
	}
    return 0;
}

// Global variables

static TCHAR szWindowClass[] = ("win32app");
static TCHAR szTitle[] = ("Heat Map");

HINSTANCE hInst;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
	
	// Pick 20 distinct regions of random sizes from 10 to 100
	// regions start from 0-900,0-900
	// regions have 0-1000 drops
	srand(time(null));
	for(int i = 0; i < 20; i++) {
		int startx = rand() % 800;
		int starty = rand() % 800;
		int endx = startx + rand() % 200 + 10;
		int endy = starty + rand() % 200 + 10;
		int drops = rand() % 100;
		for(int j = 0; j < drops; j++) {
			HeatMap.AddValue(GOLD_DROP, 1, (rand() % (endx - startx)) + startx, (rand() % (endy - starty)) + starty);
		}
	}
	/* Uncoment this for very large vertex tests
	for(int i = 0; i < 10000000; i++) {
		HeatMap.AddValue(GOLD_DROP, 1, rand() % 10000, rand() % 10000);
	}
	for(int i = 0; i < 10000000; i++) {
		HeatMap.AddValue(MONSTER_KILL, 1, rand() % 1000, rand() % 10000);
	}

	// 2 million random drops from 0,0 to 10,10
	for(int i = 0; i < 100000; i++) {
		HeatMap.AddValue(GOLD_DROP, 1, rand() % 10, rand() % 10);
	}
	for(int i = 0; i < 100000; i++) {
		HeatMap.AddValue(MONSTER_KILL, 1, rand() % 10, rand() % 10);
	}
	*/
	
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            ("Call to RegisterClassEx failed!"),
            ("Heat Map"),
            NULL);

        return 1;
    }

    hInst = hInstance;

    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 1000,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            ("Call to CreateWindow failed!"),
            ("Heat Map"),
            NULL);

        return 1;
    }

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(hWnd,
        nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}