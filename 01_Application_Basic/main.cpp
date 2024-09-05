#include "Application.h"

int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	Application app;

	app.Initialize(hInstance, nCmdShow);
	app.Loop();
	app.Uninitialize();

	return 0;
}
