#include <Windows.h> //This is header file.  Contains win32
#include <gl/GL.h>
#include <gl/GLU.h>
#include <stdio.h>
#pragma comment (lib,"openGL32.lib")
#pragma comment (lib,"glu32.lib")

// Project 1 -> Properties -> Linker -> System -> SubSystem

// function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int initialize();
void resize(int width, int height);
void display();


//GlobalFunctions
HWND ghwnd = NULL;
DWORD dwStyle;
BOOL bFullScreen = FALSE;
WINDOWPLACEMENT wpprev = { sizeof(WINDOWPLACEMENT) };
bool bDone;
bool gbActiveWindow;
HDC gHdc = NULL;
HGLRC ghglrc = NULL; // openGL renderer context
FILE* gpLogFile = NULL; // for error checking
bool fullscreen = false;
BOOL blsWindowPlacement;
HMONITOR hMonitor;
BOOL blsMonitorInfo;
float zValue = -3.0f;

void toggleFullscreen(void);



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) //Entry Point Function
{

	// checks for log file and creates one if one does not exist
	if (fopen_s(&gpLogFile, "Log.txt", "w") != 0) {
		MessageBox(NULL, TEXT("Log file was not created"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else {
		fprintf_s(gpLogFile, "Log file created\n");
	}


	// Three steps to make a window
	// 1. say what the window will look like and fill a structure with style details
	// 2. register the structure
	// 3. call the createwindow api and pass the class

	WNDCLASSEX wndclass;
	HWND hwnd = NULL;
	MSG msg;
	TCHAR szAppName[] = TEXT("Kyle's App");
	int result = 0;



	// step 1
	wndclass.cbSize = sizeof(WNDCLASSEX); //giving the memory to this structure. 
	wndclass.style = CS_HREDRAW | CS_VREDRAW; // makes the window horizontally and vertically resize
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// step 2
	RegisterClassEx(&wndclass);

	// step 3
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Kyle's Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		CW_USEDEFAULT, // starting X
		CW_USEDEFAULT, //starting Y
		CW_USEDEFAULT, //Width
		CW_USEDEFAULT, //Height
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;
	result = initialize();

	// takes potential errors from the initialize and logs them log file
	if (result == -1) {
		fprintf_s(gpLogFile, "Choose pixel format failed\n");
		DestroyWindow(hwnd);
	}
	else if (result == -2) {
		fprintf_s(gpLogFile, "Set pixel format failed\n");
		DestroyWindow(hwnd);
	}
	else if (result == -3) {
		fprintf_s(gpLogFile, "WGL create context failed\n");
		DestroyWindow(hwnd);
	}
	else if (result == -4) {
		fprintf_s(gpLogFile, "WGL make current failed\n");
		DestroyWindow(hwnd);
	}
	ShowWindow(hwnd, SW_SHOWNORMAL);
	SetFocus(hwnd);
	SetForegroundWindow(hwnd);
	UpdateWindow(hwnd);
	toggleFullscreen();



	//Heart of the Application
	//Message Loop
	//while (GetMessage(&msg, NULL,/*Filter*/ 0, 0))
	//{
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//}

	//main graphics loop
	while (bDone == false)
	{
		// checks for keydown or events
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg); // converts msg
				DispatchMessage(&msg); // calls wndproc()
			}
		}
		// render
		else
		{
			if (gbActiveWindow == true)
			{
				// here render everything
				display();
			}
			else
			{


			}
		}
	}
	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

	TCHAR STR[80] = TEXT("Hello world");
	HDC hdc;
	RECT rc;
	static int width = 0;
	static int height = 0;

	switch (iMsg)
	{


	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'V':
		case 'v':
			MessageBox(hwnd, TEXT("Hello world"), TEXT("This is a message box"), MB_OK);
			break;

		case 'S':
		case 's':
			MessageBox(hwnd, TEXT("Hello world"), TEXT("This is a message!"), MB_OK);
			break;
		case 'F': // ascii value for 'f' is the same as the numpad 6, for some reason
		case 'f':
			toggleFullscreen();
			break;
		case 'w':
		case 'W':
			zValue -= 0.05f;
			break;
		case 'x':
		case 'X':
			zValue += 0.05f;
			break;
			
		}

		break;

		// look into HIWORD and LOWORD, and lparam and GLsizei
	case WM_SIZE: // when the window gets resized, the OS calls this case
		resize(LOWORD(lParam), HIWORD(lParam));
		height = HIWORD(lParam);
		width = LOWORD(lParam);
		break;
		/*case WM_PAINT:
			GetClientRect(ghwnd, &rc);
			hdc = GetDC(ghwnd);
			SetBkColor(hdc, RGB(0, 0, 0));
			SetTextColor(hdc, RGB(0, 255, 0));
			DrawText(hdc, STR, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			ReleaseDC(ghwnd, hdc);
			break;*/

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_CREATE: // called before the window is created, like a constructor kind of
		MessageBox(hwnd, TEXT("This is a message box :)"), TEXT("Hello world"), MB_OK);
		break;

	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

int initialize() {
	// defining how we want the pixel to the OS
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;
	memset((void*)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));  // go over this
	pfd.nVersion = 1; // initialize the struct with the minimum OpenGL version to 1.1
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	// being done by the OS
	gHdc = GetDC(ghwnd); // gets the device context from the OS
	iPixelFormatIndex = ChoosePixelFormat(gHdc, &pfd); // the OS already has a bunch of pixel format descriptors, this function takes yours and maps it to one the OS has, and returns an index to it
	if (iPixelFormatIndex == 0) { // if the OS doesn't have your pfd structure, return an error
		return -1;
	}
	if (SetPixelFormat(gHdc, iPixelFormatIndex, &pfd) == FALSE) { // links ghdc with your pfd structure
		return -2;
	}

	// done by openGL
	ghglrc = wglCreateContext(gHdc); // creates a context on the GPU side and map ghdc to ghglrc from the cpu side
	if (ghglrc == NULL) {
		return -3;
	}

	if (wglMakeCurrent(gHdc, ghglrc) == FALSE) { // makes current painter as ghglrc and replacing ghdc to ghglrc
		return -4;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // colors the window with opengl color
	resize(800, 600);
	return 0;
}

void resize(int width, int height) {
	if (height == 0) {
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height); // total screen that will render
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

}

void display() {
	static float angle = 0.0f;
	glClear(GL_COLOR_BUFFER_BIT); // telling opengl to color the screen with the color that is in opengl's current state (in this case, blue from glclearcolor)
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glTranslatef(-1.3f, 0.0f, -6.0f);
	/*glScalef(0.5f, 0.5f, 0.5f);
	glRotatef(angle, 1.0f, 1.0f, 1.0f);*/
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(1.0f, -1.0f);
	glEnd();

	glLoadIdentity();
	glTranslatef(1.3f, 0.0f, -6.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, -1.0, 0.0f);
	glEnd();
	
	glLoadIdentity();
	glTranslatef(-1.0f, 0.0f, zValue);
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	SwapBuffers(gHdc); // swap because we're using double buffering, how many times we swap is dependent on how many fps 
	angle += 0.1f;

}

void uninitilize() {
	if (wglGetCurrentContext() == ghglrc) {
		wglMakeCurrent(NULL, NULL);
		if (ghglrc) {
			wglDeleteContext(ghglrc);
			ghglrc = NULL;
		}
	}
	if (gHdc) {
		ReleaseDC(ghwnd, gHdc);
		gHdc = NULL;
	}
}

void toggleFullscreen(void) {
	MONITORINFO mi;
	if (fullscreen == false) {
		mi.cbSize = { sizeof(MONITORINFO) };
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE); // get the style from the OS about the current window, will return WS_OVERLAPPEDWINDOW (the current window style)
		if (dwStyle && WS_OVERLAPPEDWINDOW) { // will return true if the new style is WS_OVERLAPPEDWINDOW
			blsWindowPlacement = GetWindowPlacement(ghwnd, &wpprev); // the window's position gets restored back from leaving fullscreen
			hMonitor = MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY); // contains handle to your monitor's screen
			blsMonitorInfo = GetMonitorInfo(hMonitor, &mi); // gets monitor info
			if (blsWindowPlacement == TRUE && blsMonitorInfo == TRUE) {
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW); // setting the new style of the window and removes the WS_OVERLAPPEDWINDOW from dsStyle
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		fullscreen = true;
	}
	else {
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW); // re-adds ws_overlappedwindow back to the dwstyle when we exit the fullscreen by making the or statement to act as a bitwise operator
		SetWindowPlacement(ghwnd, &wpprev); // restores old window placement
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
		fullscreen = false;
		ShowCursor(TRUE);
	}
}