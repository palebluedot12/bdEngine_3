#include "FBXLoading.h"
#include "..\\Engine\\Helper.h"
#include <d3dcompiler.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <Directxtk/DDSTextureLoader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <DirectXTex.h>
#include "..\Engine\Imguizmo\ImGuizmo.h"
#include <imgui_internal.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

FBXLoading::FBXLoading(HINSTANCE hInstance)
	:GameApp(hInstance)
	, m_LightDirection(0.0f, 0.0f, -1.0f)
	, m_LightAmbient(0.0f, 0.0f, 0.0f, 1.0f)
	, m_LightDiffuse(1.0f, 1.0f, 1.0f, 1.0f)
	, m_LightSpecular(1.0f, 1.0f, 1.0f, 1.0f)
	, m_CameraPos(0.0f, 100.0f, 450.0f)
	, m_CameraDirection(0.0f, 0.0f, -1.0f)
	, m_CameraMoveSpeed(100.0f)
	, m_MaterialAmbient(1.0f, 1.0f, 1.0f, 1.0f)
	, m_MaterialDiffuse(1.0f, 1.0f, 1.0f, 1.0f)
	, m_MaterialSpecular(1.0f, 1.0f, 1.0f, 1.0f)
	, m_MaterialSpecularPower(2000.0f)
	, m_bSpecularMapEnabled(true)
	, m_ModelMatrix(XMMatrixIdentity())
	, m_ModelScale(1.0f, 1.0f, 1.0f)
	, m_camDistance(300.0f)
	, m_MouseSensitivity(0.001f)
{

}

FBXLoading::~FBXLoading()
{
	UninitScene();
	UninitD3D();
}

bool FBXLoading::Initialize(UINT Width, UINT Height)
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

void FBXLoading::Update()
{
	__super::Update();

	float t = GameTimer::m_Instance->TotalTime();
	float deltaTime = GameTimer::m_Instance->DeltaTime();
	float moveSpeed = m_CameraMoveSpeed * deltaTime;

	m_FPS = 1.0f / deltaTime;

	XMVECTOR cameraDirection = XMVector3Normalize(XMLoadFloat3(&m_CameraDirection));

	// 카메라 움직이기
	if (GetAsyncKeyState('W') & 0x8000)
	{
		XMStoreFloat3(&m_CameraPos, XMLoadFloat3(&m_CameraPos) + cameraDirection * moveSpeed); // XMVECTOR를 XMFLOAT3로 변환 후 저장
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		//XMVECTOR cameraDirection = XMVector3Normalize(XMLoadFloat3(&m_CameraDirection));
		XMStoreFloat3(&m_CameraPos, XMLoadFloat3(&m_CameraPos) + cameraDirection * -moveSpeed); 
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		XMVECTOR cameraRight = XMVector3Cross(XMLoadFloat3(&m_CameraDirection), XMVectorSet(0, 1, 0, 0));
		XMStoreFloat3(&m_CameraPos, XMLoadFloat3(&m_CameraPos) + XMVector3Normalize(cameraRight) * moveSpeed); 
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		XMVECTOR cameraRight = XMVector3Cross(XMLoadFloat3(&m_CameraDirection), XMVectorSet(0, 1, 0, 0));
		XMStoreFloat3(&m_CameraPos, XMLoadFloat3(&m_CameraPos) + XMVector3Normalize(cameraRight) * -moveSpeed);
	}
	if (GetAsyncKeyState('Q') & 0x8000)
		m_CameraPos.y += moveSpeed;
	if (GetAsyncKeyState('E') & 0x8000)
		m_CameraPos.y -= moveSpeed;

	// 주시점 고정된 LookAt
	//XMMATRIX view = XMMatrixLookAtLH(
	//	XMLoadFloat3(&m_CameraPos),			// 카메라 위치
	//	XMVectorSet(0, 120, 0, 1),			// 주시점
	//	XMVectorSet(0, 1, 0, 0));

	 // 마우스 이동처리
	if (GetKeyState(VK_RBUTTON) & 0x8000) {
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(m_hWnd, &p);
		int dx = p.x - m_PrevMouseX;
		int dy = p.y - m_PrevMouseY;
		// 마우스 감도 적용
		m_CameraDirection.x += -dx * m_MouseSensitivity;
		m_CameraDirection.y += -dy * m_MouseSensitivity;
	}
	GetCursorPos(&m_MousePos);
	ScreenToClient(m_hWnd, &m_MousePos);
	m_PrevMouseX = m_MousePos.x;
	m_PrevMouseY = m_MousePos.y;

	// 카메라 같이 움직이는 LookTo
	XMMATRIX view = XMMatrixLookToLH(
		XMLoadFloat3(&m_CameraPos),
		XMLoadFloat3(&m_CameraDirection),
		XMVectorSet(0, 1, 0, 0));

	// 카메라 위치 계산 (View 역행렬)
	XMVECTOR determinant;
	XMMATRIX invView = XMMatrixInverse(&determinant, view);
	XMVECTOR cameraPos = invView.r[3];

	// 월드 공간에서의 카메라 위치
	XMStoreFloat3(&m_ViewDirEvaluated, cameraPos);

	m_FBXRenderer->SetView(view);

	//Logger::default_log_level = LOG_LEVEL::LOG_LEVEL_DEBUG;
	//Logger::write("LogFile", "This is a Test.");
}

void FBXLoading::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// Clear 
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_FBXRenderer->Render(m_AssimpLoader->GetMeshes(), m_AssimpLoader->GetMaterials(), m_ViewDirEvaluated);

	// ImGui rendering
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// ImGui 창 시작
	ImGui::Begin("FBX Model");

	// 여기 ImGuizmo
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetOrthographic(false); // !isPerspective
	ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
	ImGuizmo::BeginFrame();

	Matrix viewMatrix;
	Matrix projectionMatrix;

	viewMatrix = m_FBXRenderer->GetView();
	projectionMatrix = m_FBXRenderer->GetProjection();

	for (int i = 0; i < m_ModelInstances.size(); ++i)
	{
		if (ImGui::RadioButton(("model" + std::to_string(i)).c_str(), m_CurrentGizmoTarget == i))
			m_CurrentGizmoTarget = i;

		if (i != m_CurrentGizmoTarget)
			continue;

		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(*m_ModelInstances[i].modelMatrix.m, matrixTranslation, matrixRotation, matrixScale);

		XMMATRIX scaleMatrix = XMMatrixScaling(m_ModelScale.x, m_ModelScale.y, m_ModelScale.z);

		XMMATRIX baseTransform = scaleMatrix;
		XMFLOAT4X4 baseTransformF;
		XMStoreFloat4x4(&baseTransformF, baseTransform);
		memcpy(m_ModelInstances[i].modelMatrix.m, &baseTransformF, sizeof(float) * 16);

		ImGuizmo::Manipulate(*viewMatrix.m, *projectionMatrix.m,
			ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, *m_ModelInstances[i].modelMatrix.m);

		// Transform 결과 값을 Model Scale에 저장
		ImGuizmo::DecomposeMatrixToComponents(*m_ModelInstances[i].modelMatrix.m, matrixTranslation, matrixRotation, matrixScale);
		m_ModelScale = Vector3(matrixScale[0], matrixScale[1], matrixScale[2]);

	}

	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, m_ClientWidth, m_ClientHeight);

	float viewManipulateRight = io.DisplaySize.x;
	float viewManipulateTop = 0;
	ImGuizmo::ViewManipulate((float*)viewMatrix.m, m_camDistance, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);

	// ImGuizmo 조작
	ImGuizmo::Manipulate(*viewMatrix.m, *projectionMatrix.m, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, *m_ModelMatrix.m);

	// 라이트 조정
	ImGui::Text("Light Properties");
	ImGui::SliderFloat3("Light Direction", &m_LightDirection.x, -1.0f, 1.0f);
	ImGui::ColorEdit3("Light Ambient", &m_LightAmbient.x);
	ImGui::ColorEdit3("Light Diffuse", &m_LightDiffuse.x);
	ImGui::ColorEdit3("Light Specular", &m_LightSpecular.x);

	// Material 조정
	ImGui::Text("Material Properties");
	ImGui::ColorEdit3("Material Ambient", &m_MaterialAmbient.x);
	ImGui::ColorEdit3("Material Diffuse", &m_MaterialDiffuse.x);
	ImGui::ColorEdit3("Material Specular", &m_MaterialSpecular.x);
	ImGui::SliderFloat("Material Specular Power", &m_MaterialSpecularPower, 2.0f, 4096.0f);

	// 카메라 조정
	ImGui::Text("Camera Position");
	ImGui::SliderFloat3("Camera Position", &m_CameraPos.x, -200.0f, 200.0f);

	ImGui::Checkbox("Enable Normal Map", &boolbuffer.useNormalMap);
	ImGui::Checkbox("Enable Specular Map", &m_bSpecularMapEnabled);

	ImGui::Text("FPS: %.2f", m_FPS); // FPS 출력

	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_FBXRenderer->SetLightDirection(m_LightDirection);
	m_FBXRenderer->SetLightAmbient(m_LightAmbient);
	m_FBXRenderer->SetLightDiffuse(m_LightDiffuse);
	m_FBXRenderer->SetLightSpecular(m_LightSpecular);
	m_FBXRenderer->SetMaterialAmbient(m_MaterialAmbient);
	m_FBXRenderer->SetMaterialDiffuse(m_MaterialDiffuse);
	m_FBXRenderer->SetMaterialSpecular(m_MaterialSpecular);
	m_FBXRenderer->SetMaterialSpecularPower(m_MaterialSpecularPower);
	m_FBXRenderer->SetUseNormalMap(boolbuffer.useNormalMap);
	m_FBXRenderer->SetSpecularMapEnabled(m_bSpecularMapEnabled);

	
	auto meshes = m_AssimpLoader->GetMeshes();
	if (!meshes.empty()) // 만약 모델이 있다면, transform 변경
	{
		for (auto mesh : meshes) 
		{
			Matrix matrix = mesh->GetTransform();
			mesh->SetTransform(m_ModelMatrix);
		}
	}
		
	m_pSwapChain->Present(0, 0);
}

bool FBXLoading::InitD3D()
{
	HRESULT hr = 0;	// 결과값.

	// 스왑체인 속성 설정 구조체 생성.
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hWnd;	// 스왑체인 출력할 창 핸들 값.
	swapDesc.Windowed = true;		// 창 모드 여부 설정.
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
	// 1. 장치 생성.   2.스왑체인 생성. 3.장치 컨텍스트 생성.
	HR_T(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL,
		D3D11_SDK_VERSION, &swapDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext));

	// 4. 렌더타겟뷰 생성.  (백버퍼를 이용하는 렌더타겟뷰)	
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));
	HR_T(m_pDevice->CreateRenderTargetView(pBackBufferTexture, NULL, &m_pRenderTargetView));  // 텍스처는 내부 참조 증가
	SAFE_RELEASE(pBackBufferTexture);	//외부 참조 카운트를 감소시킨다.
	// 렌더 타겟을 최종 출력 파이프라인에 바인딩합니다.
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	//5. 뷰포트 설정.	
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

void FBXLoading::UninitD3D()
{
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pRenderTargetView);
}

bool FBXLoading::InitImGUI()
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

void FBXLoading::UninitImGUI()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK FBXLoading::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	return __super::WndProc(hWnd, message, wParam, lParam);
}


bool FBXLoading::InitScene()
{
	HRESULT hr = 0; // 결과값.

	m_AssimpLoader = new AssimpLoader(m_pDevice, m_pDeviceContext);
	m_FBXRenderer = new FBXRenderer(m_pDevice, m_pDeviceContext, m_ClientWidth, m_ClientHeight);

	//// Load Model
	//if (!m_AssimpLoader->LoadModel("../Resource/Character.fbx")) {
	//	return false;
	//}

	//if (!m_AssimpLoader->LoadModel("../Resource/zeldaPosed001.fbx")) {
	//	return false;
	//}

	m_ModelInstances.push_back({ "../Resource/zeldaPosed001.fbx", XMMatrixIdentity() });
	m_ModelInstances.push_back({ "../Resource/Character.fbx", XMMatrixTranslation(500,0,0) });

	// Load Model 할 때 행렬을 같이 넘겨줘서 위치를 조정할 수 있게 함.
	for (auto& modelInstance : m_ModelInstances) {
		if (!m_AssimpLoader->LoadModel(modelInstance.filePath, modelInstance.modelMatrix)) 
		{
			return false;
		}
	}

	m_FBXRenderer->SetMeshTextures(m_AssimpLoader->GetMeshTextures());

	// 카메라 뷰행렬 초기값설정
	XMVECTOR Eye = XMVectorSet(0.0f, 150.0f, -100.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 300.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	Matrix view = XMMatrixLookAtLH(Eye, At, Up);
	m_FBXRenderer->SetView(view);

	// (XM_PIDIV4 : 시야각 45도, 화면 비율, near z, far z 순서)
	Matrix projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.01f, 1000.0f);
	m_FBXRenderer->SetProjection(projection);

	m_ModelMatrix = XMMatrixIdentity();

	return true;
}

void FBXLoading::UninitScene()
{
	if (m_FBXRenderer) 
	{
		delete m_FBXRenderer;
		m_FBXRenderer = nullptr;
	}

	if (m_AssimpLoader)
	{
		delete m_AssimpLoader;
		m_AssimpLoader = nullptr;
	}

}