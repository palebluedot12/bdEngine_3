#pragma once
class GameApp
{
public:
	GameApp();
	BOOL InitInstance(HINSTANCE, int);

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
public:
	int Initialize(HINSTANCE hInstance, int nCmdShow);
	void Uninitialize();
	void Loop();

	void Init();
	void FixedUpdate();
	void Update();
	void LateUpdate();
	void Render();

protected:
	HINSTANCE hInst{};
	HWND hWnd{};
};

