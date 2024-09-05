#include "pch.h"
#include "GameApp.h"

GameApp::GameApp()
{

}

BOOL GameApp::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	SIZE clientSize = { 1280, 720 };
	RECT clientRect = { 0, 0, clientSize.cx, clientSize.cy };
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindow(L"Square's Dream", L"Square's Dream", WS_OVERLAPPEDWINDOW,
		0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
		nullptr, nullptr, hInstance, nullptr);
	DWORD error = GetLastError();

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK GameApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	//case WM_KEYDOWN:
	//	if (wParam == VK_F11)
	//	{
	//		// Toggle fullscreen mode
	//		if (D2DRenderer::IsWindowFullscreen(hWnd))
	//			D2DRenderer::ExitFullscreen(hWnd);
	//		else
	//			D2DRenderer::EnterFullscreen(hWnd);
	//	}
	//	break;
	//case WM_COMMAND:
	//{
	//	int wmId = LOWORD(wParam);
	//	// 메뉴 선택을 구문 분석합니다:
	//	switch (wmId)
	//	{
	//	case IDM_EXIT:
	//		DestroyWindow(hWnd);
	//		break;
	//	default:
	//		return DefWindowProc(hWnd, message, wParam, lParam);
	//	}
	//}
	//break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int GameApp::Initialize(_In_ HINSTANCE hInstance, _In_ int nCmdShow)
{

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
	wcex.lpszClassName = L"Square's Dream";
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	RegisterClassExW(&wcex);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// 얘 
	if (!D2DRenderer::Get()->InitDirect2D(hWnd))
		return FALSE;

	Init();

	return TRUE;
}

void GameApp::Uninitialize()
{
	D2DRenderer::Get()->UninitDirect2D();
}


void GameApp::Loop()
{
	MSG msg;
	// 기본 메시지 루프입니다:
	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			//윈도우 메시지 처리 
			TranslateMessage(&msg); // 키입력관련 메시지 변환  WM_KEYDOWN -> WM_CHAR
			DispatchMessage(&msg);
		}
		else
		{
			TimeManager::Get()->Update();
			Input::Update();

			FixedUpdate();
			Update();
			LateUpdate();
			Render();
		}
	}
}

void GameApp::Init()
{
	TimeManager::Get()->Init();
	Input::Initailize();
	D2DRenderer::Get()->InitDirect2D(hWnd);
}

void GameApp::FixedUpdate()
{
	static float deltaCount;
	deltaCount += TimeManager::Get()->GetDeltaTime();
	while (deltaCount >= 0.02f)
	{
		//CollisionManager 추가

		WorldManager::FixedUpdate();
		deltaCount -= 0.02f;
	}

}

void GameApp::Update()
{
	//TimeManager::Get()->Update();
	WorldManager::Update();
	//Input::Update();
	CollisionManager::Update();
}

void GameApp::LateUpdate()
{

}

void GameApp::Render()
{
	D2DRenderer::Get()->BeginDraw();
	WorldManager::Render();
	D2DRenderer::Get()->EndDraw();

}