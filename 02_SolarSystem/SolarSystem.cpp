﻿#include "SolarSystem.h"
#include "..\\Engine\\Helper.h"
#include <d3dcompiler.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <DirectXTK/SimpleMath.h>
#include <DirectXTK/Model.h>
#include <DirectXTK/Effects.h>
#include <DirectXTK/CommonStates.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
//#pragma comment(lib, "assimp-vc142-mt.lib") // Assimp 라이브러리 링크

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


// 정점 선언.
struct Vertex
{
	Vector3 position;		// 정점 위치 정보.
	Vector4 color;			// 정점 색상 정보.

	Vertex() { }
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

SolarSystem::SolarSystem(HINSTANCE hInstance)
	:GameApp(hInstance)
{

}

SolarSystem::~SolarSystem()
{
	UninitScene();
	UninitD3D();
}

bool SolarSystem::Initialize(UINT Width, UINT Height)
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

void SolarSystem::Update()
{
	__super::Update();

	float t = GameTimer::m_Instance->TotalTime();

	// XM = DirectXMath, y축을 중심으로 회전하는 행렬. 1번째 큐브
	m_World1 = XMMatrixRotationY(t);

	// 2번째 큐브.
	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	XMMATRIX mSpin = XMMatrixRotationZ(-t);
	XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	XMMATRIX mOrbit = XMMatrixRotationY(-t * 2.0f);

	m_World2 = mScale * mSpin * mTranslate * mOrbit; // 스케일적용 -> R(제자리Y회전) -> 왼쪽으로 이동 ->  궤도회전  

}

void SolarSystem::Render()
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


	//// Update variables for the first cube
	//ConstantBuffer cb1;
	//cb1.mWorld = XMMatrixTranspose(m_World1);
	//cb1.mView = XMMatrixTranspose(m_View);
	//cb1.mProjection = XMMatrixTranspose(m_Projection);
	//m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

	//m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);


	//// Update variables for the second cube	
	//ConstantBuffer cb2;
	//cb2.mWorld = XMMatrixTranspose(m_World2);
	//cb2.mView = XMMatrixTranspose(m_View);
	//cb2.mProjection = XMMatrixTranspose(m_Projection);
	//m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb2, 0, 0);

	//m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);

	   // Update variables for the first cube
	ConstantBuffer cb1;
	cb1.mWorld = DirectX::XMMatrixTranspose(m_World1);
	cb1.mView = DirectX::XMMatrixTranspose(m_View);
	cb1.mProjection = DirectX::XMMatrixTranspose(m_Projection);

	// Ensure the constant buffer is valid
	if (m_pConstantBuffer)
	{
		m_pDeviceContext->UpdateSubresource(
			m_pConstantBuffer,   // Destination buffer
			0,                   // Subresource index
			nullptr,             // No need for a box
			&cb1,                // Pointer to data
			0,                   // Row pitch (not used)
			0                    // Depth pitch (not used)
		);
	}
	else
	{
		// Handle the case where m_pConstantBuffer is not initialized
		// You can add error handling or logging here
	}

	m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);


	//imgui
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (m_show_demo_window)
		ImGui::ShowDemoWindow(&m_show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &m_show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &m_show_another_window);

		ImGui::SliderFloat("float", &m_f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			m_counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", m_counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::ColorEdit3("clear color", (float*)&m_ClearColor); // Edit 3 floats representing a color	
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	// Present the information rendered to the back buffer to the front buffer (the screen)
	m_pSwapChain->Present(0, 0);
}

bool SolarSystem::InitD3D()
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
	// 렌더 타겟을 최종 출력 파이프라인에 바인딩합니다.
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

	//6. 뎊스&스텐실 뷰 생성
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

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	return true;
}

void SolarSystem::UninitD3D()
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
bool SolarSystem::InitScene()
{
	HRESULT hr = 0; // 결과값.
	
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile("turtle2.fbx", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (!scene || !scene->HasMeshes()) {
		MessageBox(0, L"FBX 파일을 불러오는 데 실패했습니다.", 0, 0);
		return false;
	}

	// 첫 번째 메쉬
	aiMesh* mesh = scene->mMeshes[0];

	std::vector<Vertex> vertices;
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) 
	{
		Vertex vertex;
		vertex.position = Vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

		// 색상 정보가 있는 경우 처리 (없으면 기본 색상 적용)
		if (mesh->HasVertexColors(0)) {
			vertex.color = Vector4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
		}
		else {
			vertex.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 기본 색상 (하얀색)
		}

		vertices.push_back(vertex);
	}

	std::vector<WORD> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) 
	{
		aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) 
		{
			indices.push_back(static_cast<WORD>(face.mIndices[j]));
		}
	}

	// Vertex Buffer 생성
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices.data();
	HR_T(m_pDevice->CreateBuffer(&bd, &vbData, &m_pVertexBuffer));

	m_VertexBufferStride = sizeof(Vertex);
	m_VertexBufferOffset = 0;

	// Index Buffer 생성
	bd.ByteWidth = sizeof(WORD) * indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices.data();
	HR_T(m_pDevice->CreateBuffer(&bd, &ibData, &m_pIndexBuffer));

	m_nIndices = indices.size();

	// 4. Input Layout 생성 (셰이더에서 사용할 정점 레이아웃 정의)
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	ID3D10Blob* vertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout));

	// 5. Vertex Shader 생성
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));
	SAFE_RELEASE(vertexShaderBuffer);

	// 6. Pixel Shader 생성
	ID3D10Blob* pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader));
	SAFE_RELEASE(pixelShaderBuffer);

	// 7. Constant Buffer 생성 (월드, 뷰, 프로젝션 행렬)
	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pConstantBuffer));

	// 초기 행렬 데이터 설정 (월드, 뷰, 프로젝션)
	m_World1 = XMMatrixIdentity();
	m_World2 = XMMatrixIdentity();
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -200.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, m_ClientWidth / (FLOAT)m_ClientHeight, 0.01f, 500.0f);

	return true;
}

void SolarSystem::UninitScene()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pDepthStencilView);
}

bool SolarSystem::InitImGUI()
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

void SolarSystem::UninitImGUI()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK SolarSystem::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	return __super::WndProc(hWnd, message, wParam, lParam);
}
