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

// 정점 선언.
struct Vertex
{
	Vector3 position;		// 정점 위치 정보.
	Vector4 color;			// 정점 색상 정보.

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

	// 렌더링 파이프라인을 구성하는 필수 객체의 인터페이스
	ID3D11Device* m_pDevice = nullptr;						// 디바이스	
	ID3D11DeviceContext* m_pDeviceContext = nullptr;		// 즉시 디바이스 컨텍스트
	IDXGISwapChain* m_pSwapChain = nullptr;					// 스왑체인
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;	// 렌더링 타겟뷰

	//depthstencil
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;  
	ID3D11Texture2D* m_DepthStencilBuffer = nullptr;
	ID3D11DepthStencilState* m_DepthStencilState = nullptr;

	// 렌더링 파이프라인에 적용하는  객체와 정보
	ID3D11VertexShader* m_pVertexShader = nullptr;	// 정점 셰이더.
	ID3D11PixelShader* m_pPixelShader = nullptr;	// 픽셀 셰이더.	
	ID3D11InputLayout* m_pInputLayout = nullptr;	// 입력 레이아웃.
	ID3D11Buffer* m_pVertexBuffer = nullptr;		// 버텍스 버퍼.
	UINT m_VertexBufferStride = 0;					// 버텍스 하나의 크기.
	UINT m_VertexBufferOffset = 0;					// 버텍스 버퍼의 오프셋.
	ID3D11Buffer* m_pIndexBuffer = nullptr;			// 버텍스 버퍼.
	int m_nIndices = 0;								// 인덱스 개수.
	ID3D11Buffer* m_pConstantBuffer = nullptr;		// 상수 버퍼.

	//imgui용 변수들
	Vector4 m_ClearColor = Vector4(0.45f, 0.55f, 0.60f, 1.00f);
	bool m_show_another_window = false;
	bool m_show_demo_window = true;
	float m_f;
	int m_counter;

	// 쉐이더에 전달할 데이터
	Matrix                m_World1;				// 월드좌표계 공간으로 변환을 위한 행렬.
	Matrix                m_World2;				// 월드좌표계 공간으로 변환을 위한 행렬.
	Matrix                m_View;				// 카메라좌표계 공간으로 변환을 위한 행렬.
	Matrix                m_Projection;			// 단위장치좌표계( Normalized Device Coordinate) 공간으로 변환을 위한 행렬.

	virtual bool Initialize(UINT Width, UINT Height);
	virtual void Update();
	virtual void Render();

	bool InitD3D();
	void UninitD3D();

	bool InitImGUI();
	void UninitImGUI();

	bool InitScene();		// 쉐이더,버텍스,인덱스
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

