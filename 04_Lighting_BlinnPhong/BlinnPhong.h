#pragma once
#include <windows.h>
#include "..\\Engine\\GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>


using namespace DirectX::SimpleMath;
using namespace DirectX;

class Lambertian :
	public GameApp
{
public:
	Lambertian(HINSTANCE hInstance);
	~Lambertian();

	// ������ ������������ �����ϴ� �ʼ� ��ü�� �������̽� 
	ID3D11Device* m_pDevice = nullptr;						// ����̽�	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// ��� ����̽� ���ؽ�Ʈ
	IDXGISwapChain* m_pSwapChain = nullptr;					// ����ü��
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// ������ Ÿ�ٺ�
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;  // ���̰� ó���� ���� �X�����ٽ� ��

	// ������ ���������ο� �����ϴ�  ��ü�� ����
	ID3D11VertexShader* m_pVertexShader = nullptr;		// ���� ���̴�.
	ID3D11PixelShader* m_pPixelShader = nullptr;		// �ȼ� ���̴�.	
	ID3D11PixelShader* m_pPixelShaderSolid = nullptr;	// �ȼ� ���̴� ����Ʈ ǥ�ÿ�.	
	ID3D11InputLayout* m_pInputLayout = nullptr;		// �Է� ���̾ƿ�.
	ID3D11Buffer* m_pVertexBuffer = nullptr;			// ���ؽ� ����.
	UINT m_VertexBufferStride = 0;						// ���ؽ� �ϳ��� ũ��.
	UINT m_VertexBufferOffset = 0;						// ���ؽ� ������ ������.
	ID3D11Buffer* m_pIndexBuffer = nullptr;				// ���ؽ� ����.
	int m_nIndices = 0;									// �ε��� ����.
	ID3D11Buffer* m_pConstantBuffer = nullptr;			// ��� ����.
	ID3D11ShaderResourceView* m_pTextureRV = nullptr;	// �ؽ�ó ���ҽ� ��.
	ID3D11SamplerState* m_pSamplerLinear = nullptr;		// ���÷� ����.


	Matrix                m_World;				// ������ǥ�� �������� ��ȯ�� ���� ���.
	Matrix                m_View;				// ����ǥ�� �������� ��ȯ�� ���� ���.
	Matrix                m_Projection;			// ������ġ��ǥ��( Normalized Device Coordinate) �������� ��ȯ�� ���� ���.

	XMFLOAT4 m_LightColors[2] =
	{
		XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
		XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f)
	};
	XMFLOAT4 m_InitialLightDirs[2] =
	{
		XMFLOAT4(-0.577f, 0.577f, -1.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
	};
	XMFLOAT4 m_LightDirsEvaluated[2] = {};		// ���� ����Ʈ ����

	float m_Yaw = 0.0f;     // Yaw
	float m_Pitch = 0.0f;   // ȸ���� X�� (Pitch)

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

