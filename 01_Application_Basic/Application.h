#pragma once
#include <windows.h>
#include "..\\Engine\\GameApp.h"
#include <d3d11.h>
#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;
using namespace DirectX;

struct Mesh
{
	Matrix mLocal;
	Matrix mWorld;
	float rotationSpeed;
};

// ���� ����.
struct Vertex
{
	Vector3 position;		// ���� ��ġ ����.
	Vector4 color;			// ���� ���� ����.

	Vertex(float x, float y, float z) : position(x, y, z) { }
	Vertex(Vector3 position) : position(position) { }

	Vertex(Vector3 position, Vector4 color)
		: position(position), color(color) { }
};

struct ConstantBuffer
{
	Matrix mWorld;
	Matrix mView;
	Matrix mProjection;
};

class Application :
	public GameApp
{
public:
	Application(HINSTANCE hInstance);
	~Application();

	// ������ ������������ �����ϴ� �ʼ� ��ü�� �������̽�
	ID3D11Device* m_pDevice = nullptr;						// ����̽�	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// ��� ����̽� ���ؽ�Ʈ
	IDXGISwapChain* m_pSwapChain = nullptr;					// ����ü��
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// ������ Ÿ�ٺ�

	//depthstencil
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;  
	ID3D11Texture2D* m_DepthStencilBuffer = nullptr;
	ID3D11DepthStencilState* m_DepthStencilState = nullptr;

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

private:
	Mesh m_Meshes[3];
	Vector3 m_MeshPositions[3];
	Vector3 m_CameraPosition;
	float m_CameraFOV;
	float m_CameraNear;
	float m_CameraFar;

	bool InitDepthStencil();


};

