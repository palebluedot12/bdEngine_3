#pragma once
#include <windows.h>
#include "..\\Engine\\GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <DirectXTK/Model.h>
#include <DirectXTK/Effects.h>
#include <DirectXTK/CommonStates.h>

using namespace DirectX::SimpleMath;
using namespace DirectX;

class SolarSystem :
	public GameApp
{
public:
	SolarSystem(HINSTANCE hInstance);
	~SolarSystem();

	// ������ ������������ �����ϴ� �ʼ� ��ü�� �������̽� (  �X�� ���ٽ� �䵵 ������ ���� ������� �ʴ´�.)
	ID3D11Device* m_pDevice = nullptr;						// ����̽�	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// ��� ����̽� ���ؽ�Ʈ
	IDXGISwapChain* m_pSwapChain = nullptr;					// ����ü��
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// ������ Ÿ�ٺ�
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;  // ���̰� ó���� ���� �X�����ٽ� ��

	// ������ ���������ο� �����ϴ�  ��ü�� ����
	ID3D11VertexShader* m_pVertexShader = nullptr;	// ���� ���̴�.
	ID3D11PixelShader* m_pPixelShader = nullptr;	// �ȼ� ���̴�.	
	ID3D11InputLayout* m_pInputLayout = nullptr;	// �Է� ���̾ƿ�.
	ID3D11Buffer* m_pVertexBuffer = nullptr;		// ���ؽ� ����.
	UINT m_VertexBufferStride = 0;					// ���ؽ� �ϳ��� ũ��.
	UINT m_VertexBufferOffset = 0;					// ���ؽ� ������ ������.
	ID3D11Buffer* m_pIndexBuffer = nullptr;			// ���ؽ� ����.
	int m_nIndices = 0;								// �ε��� ����.
	ID3D11Buffer* m_pConstantBuffer = nullptr;		// ��� ����.

	//imgui�� ������
	Vector4 m_ClearColor = Vector4(0.45f, 0.55f, 0.60f, 1.00f);
	bool m_show_another_window = false;
	bool m_show_demo_window = true;
	float m_f;
	int m_counter;

	// ���̴��� ������ ������
	Matrix                m_World1;				// ������ǥ�� �������� ��ȯ�� ���� ���.
	Matrix                m_World2;				// ������ǥ�� �������� ��ȯ�� ���� ���.
	Matrix                m_View;				// ī�޶���ǥ�� �������� ��ȯ�� ���� ���.
	Matrix                m_Projection;			// ������ġ��ǥ��( Normalized Device Coordinate) �������� ��ȯ�� ���� ���.

	std::unique_ptr<DirectX::Model> m_model;
	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::BasicEffect> m_effect;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;



	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();
	void UninitD3D();

	bool InitImGUI();
	void UninitImGUI();

	bool InitScene();		// ���̴�,���ؽ�,�ε���
	void UninitScene();

	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

