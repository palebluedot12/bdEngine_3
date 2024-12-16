#pragma once

#include "..\\Engine\\GameApp.h"
#include "..\\Engine\\AssimpLoader.h"
#include "..\\Engine\\FBXRenderer.h"

class FBXLoading : public GameApp
{
public:
	FBXLoading(HINSTANCE hInstance);
	~FBXLoading();

	bool Initialize(UINT Width, UINT Height) override;
	void Update() override;
	void Render() override;

	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	bool InitD3D();
	void UninitD3D();
	bool InitScene();
	void UninitScene();
	bool InitImGUI();
	void UninitImGUI();

	AssimpLoader m_AssimpLoader;
	FBXRenderer* m_FBXRenderer;

	// Light properties
	Vector3 m_LightDirection;
	Vector4 m_LightAmbient;
	Vector4 m_LightDiffuse;
	Vector4 m_LightSpecular;

	// Material Properties
	Vector4 m_MaterialAmbient;
	Vector4 m_MaterialDiffuse;
	Vector4 m_MaterialSpecular;
	float m_MaterialSpecularPower;

	// Camera Properties
	Vector3 m_CameraPos;

	bool m_bSpecularMapEnabled;
	BoolBuffer boolbuffer;
	Vector3 m_ViewDirEvaluated;

	// D3D11 Device and Context
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
};