#include "Application.h"
#include "..\\Engine\\Helper.h"
#include <d3dcompiler.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <string>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

/*
렌더링 파이프라인
--------------------------------------------------
Input Assembler
Vertex Shader
(생략 가능) Tessellation Stage
Rasterizer (고정값)
Pixel Shader
Output Merger
*/


Application::Application(HINSTANCE hInstance)
	:GameApp(hInstance),
	m_CameraPosition(0.0f, 1.0f, -5.0f),
	m_CameraFOV(XM_PIDIV2), // PIDIV2 => 파이/2 => 90도 시야각
	m_CameraNear(0.01f),
	m_CameraFar(100.0f)
{
	m_MeshPositions[0] = Vector3(0.0f, 0.0f, 0.0f);
	m_MeshPositions[1] = Vector3(3.0f, 0.3f, 0.0f);
	m_MeshPositions[2] = Vector3(4.5f, 0.3f, 0.0f);

	m_Meshes[0].rotationSpeed = 1.0f;
	m_Meshes[1].rotationSpeed = 2.0f;
	m_Meshes[2].rotationSpeed = 0.1f;
}

Application::~Application()
{
	UninitScene();
	UninitD3D();
}

bool Application::Initialize(UINT Width, UINT Height)
{
	__super::Initialize(Width, Height);

	if (!InitD3D())
		return false;

	if (!InitScene())
		return false;

	if (!InitImGUI())
		return false;

	return true;
}

void Application::Update()
{
	__super::Update();

	float t = GameTimer::m_Instance->TotalTime();

	//// XM = DirectXMath, y축을 중심으로 회전하는 행렬. 1번째 큐브
	//m_World1 = XMMatrixRotationY(t);

	//// 2번째 큐브.
	//XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	//XMMATRIX mSpin = XMMatrixRotationZ(-t);
	//XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	//XMMATRIX mOrbit = XMMatrixRotationY(-t * 2.0f);

	//m_World2 = mScale * mSpin * mTranslate * mOrbit; // 스케일적용 -> R(제자리Y회전) -> 왼쪽으로 이동 ->  궤도회전  

	m_Meshes[0].mLocal = XMMatrixScaling(0.7f, 0.7f, 0.7f) * XMMatrixRotationY(t * m_Meshes[0].rotationSpeed) * XMMatrixTranslation(m_MeshPositions[0].x, m_MeshPositions[0].y, m_MeshPositions[0].z);
	m_Meshes[1].mLocal = XMMatrixScaling(0.6f, 0.6f, 0.6f) * XMMatrixRotationY(t * m_Meshes[1].rotationSpeed) * XMMatrixTranslation(m_MeshPositions[1].x, m_MeshPositions[1].y, m_MeshPositions[1].z);
	m_Meshes[2].mLocal = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationY(t * m_Meshes[2].rotationSpeed) * XMMatrixTranslation(m_MeshPositions[2].x, m_MeshPositions[2].y, m_MeshPositions[2].z);

	// mesh 1, 2 => mesh 0 이 부모
	m_Meshes[0].mWorld = m_Meshes[0].mLocal;
	m_Meshes[1].mWorld = m_Meshes[1].mLocal * m_Meshes[0].mWorld;
	m_Meshes[2].mWorld = m_Meshes[2].mLocal * m_Meshes[1].mWorld;

	// Eye : 카메라 자체의 position
	// At : 카메라가 바라보는 position
	XMVECTOR Eye = XMVectorSet(m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);

	// Update projection matrix
	m_Projection = XMMatrixPerspectiveFovLH(m_CameraFOV, m_ClientWidth / (FLOAT)m_ClientHeight, m_CameraNear, m_CameraFar);

}

void Application::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// 화면 칠하기.
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0); // 뎁스버퍼 1.0f로 초기화.

	// Draw계열 함수를 호출하기전에 렌더링 파이프라인에 필수 스테이지 설정을 해야한다.	
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

	for (int i = 0; i < 3; ++i)
	{
		ConstantBuffer cb;
		cb.mWorld = XMMatrixTranspose(m_Meshes[i].mWorld);
		cb.mView = XMMatrixTranspose(m_View);
		cb.mProjection = XMMatrixTranspose(m_Projection);
		m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);
	}

	// ImGui rendering
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Mesh and Camera Controls");

	// Mesh position controls
	for (int i = 0; i < 3; ++i)
	{
		ImGui::Text("Mesh %d Position", i + 1);
		ImGui::SliderFloat3(("##Mesh" + std::to_string(i + 1)).c_str(), &m_MeshPositions[i].x, -10.0f, 10.0f);
	}

	// Mesh rotation speed controls
	ImGui::Text("Mesh Rotation Speed");
	for (int i = 0; i < 3; ++i)
	{
		ImGui::SliderFloat(("##RotationSpeed" + std::to_string(i + 1)).c_str(), &m_Meshes[i].rotationSpeed, 0.0f, 10.0f);
	}

	// Camera controls
	ImGui::Text("Camera Position");
	ImGui::SliderFloat3("##CameraPos", &m_CameraPosition.x, -20.0f, 20.0f);

	ImGui::Text("Camera FOV (degrees)");
	float fovDegrees = XMConvertToDegrees(m_CameraFOV);
	if (ImGui::SliderFloat("##CameraFOV", &fovDegrees, 1.0f, 180.0f))
	{
		m_CameraFOV = XMConvertToRadians(fovDegrees);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_pSwapChain->Present(0, 0);
}

bool Application::InitD3D()
{
	HRESULT hr = 0;

	// 스왑체인 속성 설정 구조체 생성.
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;								// 백버퍼 개수
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 버퍼를 렌더링 결과를 저장하기 위한 출력버퍼로 사용하겠다.
	swapDesc.OutputWindow = m_hWnd;							// 스왑체인 출력할 창 핸들 값.
	swapDesc.Windowed = true;								// 창 모드 여부 설정.
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// 백버퍼의 포맷 설정 (32비트 색상 - RGBA 8비트씩)

	// 백버퍼(텍스처)의 가로/세로 크기 설정.
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;

	// 화면 주사율 설정.
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;

	// 샘플링 관련 설정.
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 1. 장치 생성  2.스왑체인 생성  3.장치 컨텍스트 생성
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext));

	// 4. 렌더타겟뷰 생성 (백버퍼를 이용하는 렌더타겟뷰)	
	// 렌더타겟뷰는 렌더링 파이프라인의 출력을 받을 자원(Texture)을 연결하는 역할을 한다.
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &m_pRenderTargetView));  // 텍스처는 내부 참조 증가
	SAFE_RELEASE(pBackBufferTexture);	//외부 참조 카운트를 감소시킨다.
	// 렌더 타겟을 최종 출력 파이프라인에 바인딩.
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	// 5. 뷰포트 설정.	
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_ClientWidth;
	viewport.Height = (float)m_ClientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	//6. 뎁스&스텐실 뷰 생성
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = m_ClientWidth;
	descDepth.Height = m_ClientHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	ID3D11Texture2D* textureDepthStencil = nullptr;
	HR_T(m_pDevice->CreateTexture2D(&descDepth, nullptr, &textureDepthStencil));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(textureDepthStencil, &descDSV, &m_pDepthStencilView));
	SAFE_RELEASE(textureDepthStencil);

	// Create Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	ID3D11DepthStencilState* pDepthStencilState = nullptr;
	HR_T(m_pDevice->CreateDepthStencilState(&depthStencilDesc, &pDepthStencilState));
	m_pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 1);
	SAFE_RELEASE(pDepthStencilState);

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	return true;
}

void Application::UninitD3D()
{
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pRenderTargetView);
}

// 1. Render() 에서 파이프라인에 바인딩할 버텍스 버퍼및 버퍼 정보 준비
// 2. Render() 에서 파이프라인에 바인딩할 InputLayout 생성 	
// 3. Render() 에서 파이프라인에 바인딩할 버텍스 셰이더 생성
// 4. Render() 에서 파이프라인에 바인딩할 인덱스 버퍼 생성
// 5. Render() 에서 파이프라인에 바인딩할 픽셀 셰이더 생성
// 6. Render() 에서 파이프라인에 바인딩할 상수 버퍼 생성
bool Application::InitScene()
{
	HRESULT hr = 0; // 결과값.
	// 1. Render() 에서 파이프라인에 바인딩할 버텍스 버퍼및 버퍼 정보 준비
	// Normalized Device Coordinate
	//   0-----1
	//   |    /|
	//   |  /  |                중앙이 (0,0)  왼쪽이 (-1,0) 오른쪽이 (1,0) , 위쪽이 (0,1) 아래쪽이 (0,-1)
	//   |/    |
	//	 2-----3
	// 
	// 

	Vertex vertices[] = // Local or Object or Model Space    position
	{
		{ Vector3(0.0f, 1.0f, 0.0f),	Vector4(0.0f, 0.0f, 1.0f, 1.0f) }, // 위쪽 끝점
		{ Vector3(0.5f, 0.0f, 0.5f),	Vector4(0.0f, 1.0f, 0.0f, 1.0f) }, // 앞 오른쪽
		{ Vector3(-0.5f, 0.0f, 0.5f),	Vector4(0.0f, 1.0f, 1.0f, 1.0f) }, // 앞 왼쪽
		{ Vector3(0.5f, 0.0f, -0.5f),	Vector4(1.0f, 0.0f, 0.0f, 1.0f) }, // 뒤 오른쪽
		{ Vector3(-0.5f, 0.0f, -0.5f),	Vector4(1.0f, 0.0f, 1.0f, 1.0f) }, // 뒤 왼쪽
		{ Vector3(0.0f, -1.0f, 0.0f),	Vector4(1.0f, 1.0f, 0.0f, 1.0f) }, // 아래쪽 끝점
		{ Vector3(0.0f, 0.0f, 0.0f),	Vector4(1.0f, 1.0f, 1.0f, 1.0f) }, // 중심
	};

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;			// 배열 데이터 할당.
	HR_T(m_pDevice->CreateBuffer(&bd, &vbData, &m_pVertexBuffer));

	m_VertexBufferStride = sizeof(Vertex);		// 버텍스 버퍼 정보
	m_VertexBufferOffset = 0;

	// 2. Render() 에서 파이프라인에 바인딩할 InputLayout 생성 	
	D3D11_INPUT_ELEMENT_DESC layout[] = // 입력 레이아웃.
	{   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	ID3D10Blob* vertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout));

	// 3. Render() 에서 파이프라인에 바인딩할  버텍스 셰이더 생성
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));
	SAFE_RELEASE(vertexShaderBuffer);	// 버퍼 해제.

	// 4. Render() 에서 파이프라인에 바인딩할 인덱스 버퍼 생성
	//WORD indices[] =
	//{
	//	3,1,0,  2,1,3,
	//	0,5,4,  1,5,0,
	//	3,4,7,  0,4,3,
	//	1,6,5,  2,6,1,
	//	2,7,6,  3,7,2,
	//	6,4,5,  7,4,6,
	//};

	WORD indices[] =
	{
		// 위쪽 면
		0, 1, 2, 
		0, 3, 1,  
		0, 2, 4,  
		0, 4, 3,  

		// 아래쪽 면
		5, 1, 2, 
		5, 3, 1, 
		5, 2, 4,  
		5, 4, 3,  

		// 중심부를 채우는 면 (가운데 네모 부분을 삼각형으로)
		1, 3, 2, 
		3, 4, 2  
	};

	m_nIndices = ARRAYSIZE(indices);	// 인덱스 개수 저장.

	bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&bd, &ibData, &m_pIndexBuffer));

	// 5. Render() 에서 파이프라인에 바인딩할 픽셀 셰이더 생성
	ID3D10Blob* pixelShaderBuffer = nullptr;

	HR_T(CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader));

	SAFE_RELEASE(pixelShaderBuffer);

	// 6. Render() 에서 파이프라인에 바인딩할 상수 버퍼 생성
	// Create the constant buffer
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pConstantBuffer));

	//// 쉐이더에 상수버퍼에 전달할 시스템 메모리 데이터 초기화
	//m_World1 = XMMatrixIdentity();
	//m_World2 = XMMatrixIdentity();

	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, m_ClientWidth / (FLOAT)m_ClientHeight, 0.01f, 100.0f);

	return true;
}

void Application::UninitScene()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pDepthStencilView);
}

bool Application::InitImGUI()
{
	/*
		ImGui 초기화.
	*/
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(this->m_pDevice, this->m_pDeviceContext);

	//
	return true;
}

void Application::UninitImGUI()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	return __super::WndProc(hWnd, message, wParam, lParam);
}