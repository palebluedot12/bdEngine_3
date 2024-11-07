#include "NormalMap.h"
#include "..\\Engine\\Helper.h"
#include <d3dcompiler.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <Directxtk/DDSTextureLoader.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

/*
1. Vertex Format을 3D좌표, 텍스처좌표,T B N 벡터 로 구성합니다.
2. 추가 Normal Texture를 PixelShader에서 사용할 수 있도록 C++에 SRV , HLSL에 Texture2D 를 추가합니다.
3. Diffuse Texture, Normal Texture 를 적용합니다.
4. 추가 Specular Texture를 PixelShader에서 사용할 수 있도록 C++에 SRV , HLSL에 Texture2D 를 추가합니다.
5. Specular Texture 를 적용합니다.
*/

// 정점 선언.
struct Vertex
{
	Vector3 Pos;
	Vector3 Tangent;
	Vector3 Binormal;
	Vector3 Normal;
	Vector2 Tex;
};

struct ConstantBuffer
{
	Matrix mWorld;
	Matrix mView;
	Matrix mProjection;

	Vector4 vLightDir;
	Vector4 vLightAmbient;
	Vector4 vLightDiffuse;
	Vector4 vLightSpecular;
	Vector4 vMaterialAmbient;
	Vector4 vMaterialDiffuse;
	Vector4 vMaterialSpecular;
	Vector3 vCameraPos;
	float fMaterialSpecularPower;
};

NormalMap::NormalMap(HINSTANCE hInstance)
	:GameApp(hInstance)
	, m_LightDirection(0.0f, 0.0f, 1.0f)
	, m_LightAmbient(0.5f, 0.5f, 0.5f, 1.0f)
	, m_LightDiffuse(1.0f, 1.0f, 1.0f, 1.0f)
	, m_LightSpecular(1.0f, 1.0f, 1.0f, 1.0f)
	, m_MaterialAmbient(1.0f, 1.0f, 1.0f, 1.0f)
	, m_MaterialDiffuse(1.0f, 1.0f, 1.0f, 1.0f)
	, m_MaterialSpecular(1.0f, 1.0f, 1.0f, 1.0f)
	, m_MaterialSpecularPower(2000.0f)
	, m_CubeScale(1.0f, 1.0f, 1.0f)
	, m_CubeRotation(0.0f, 0.0f, 0.0f)
{

}

NormalMap::~NormalMap()
{
	UninitScene();
	UninitD3D();
}

bool NormalMap::Initialize(UINT Width, UINT Height)
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

void NormalMap::Update()
{
	__super::Update();

	float t = GameTimer::m_Instance->TotalTime();

	// 큐브 회전 행렬
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(m_CubeRotation.x),
		XMConvertToRadians(m_CubeRotation.y),
		XMConvertToRadians(m_CubeRotation.z));

	// 큐브 스케일 행렬
	XMMATRIX scaleMatrix = XMMatrixScaling(m_CubeScale.x, m_CubeScale.y, m_CubeScale.z);

	m_World = scaleMatrix * rotationMatrix;

	// 카메라 위치 계산 (View 행렬의 역행렬에서 추출)
	XMVECTOR determinant;
	XMMATRIX invView = XMMatrixInverse(&determinant, m_View);
	XMVECTOR cameraPos = invView.r[3];

	// 월드 공간에서의 카메라 위치
	XMStoreFloat3(&m_ViewDirEvaluated, XMVector3Normalize(cameraPos));

	m_View = XMMatrixLookAtLH(
		XMLoadFloat3(&m_CameraPos),
		XMVectorSet(0, 0, 0, 1),
		XMVectorSet(0, 1, 0, 0));
}

void NormalMap::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// Clear 
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Render the cube
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &m_VertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureRV);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

	// Normal On/Off
	if (m_bNormalMapEnabled)
	{
		m_pDeviceContext->PSSetShaderResources(1, 1, &m_pNormalTextureRV);  // Bind the normal map
	}
	else
	{
		ID3D11ShaderResourceView* nullSRV = nullptr;
		m_pDeviceContext->PSSetShaderResources(1, 1, &nullSRV);  // Unbind the normal map
	}

	// SpecularMap On/Off
	if (m_bSpecularMapEnabled)
	{
		m_pDeviceContext->PSSetShaderResources(2, 1, &m_pSpecularTextureRV);  // Bind the normal map
	}
	else
	{
		ID3D11ShaderResourceView* nullSRV = nullptr;
		m_pDeviceContext->PSSetShaderResources(1, 1, &nullSRV);  // Unbind the normal map
	}

	m_pDeviceContext->PSSetSamplers(1, 1, &m_pSamplerLinear);
	m_pDeviceContext->PSSetSamplers(2, 1, &m_pSamplerLinear);

	// Update matrix variables and lighting variables
	ConstantBuffer cb1;
	cb1.mWorld = XMMatrixTranspose(m_World);
	cb1.mView = XMMatrixTranspose(m_View);
	cb1.mProjection = XMMatrixTranspose(m_Projection);
	cb1.vLightDir = XMFLOAT4(m_LightDirection.x, m_LightDirection.y, m_LightDirection.z, 1.0f);
	cb1.vLightAmbient = m_LightAmbient;
	cb1.vLightDiffuse = m_LightDiffuse;
	cb1.vLightSpecular = m_LightSpecular;
	cb1.vMaterialAmbient = m_MaterialAmbient;
	cb1.vMaterialDiffuse = m_MaterialDiffuse;
	cb1.vMaterialSpecular = m_MaterialSpecular;
	cb1.fMaterialSpecularPower = m_MaterialSpecularPower;
	cb1.vCameraPos = m_ViewDirEvaluated;

	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb1, 0, 0);
	m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);

	// Calculate the light transformation matrix
	XMMATRIX mLight = XMMatrixTranslationFromVector(5.0f * XMLoadFloat3(&m_LightDirection)); // 조명 방향에 맞게 위치 조정
	XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f); // 스케일 설정
	mLight = mLightScale * mLight;

	// Update the world variable to reflect the current light
	cb1.mWorld = XMMatrixTranspose(mLight);
	//cb1.vOutputColor = m_LightDiffuse; // 현재 조명의 색상 적용
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

	// Set the pixel shader for solid color rendering
	m_pDeviceContext->PSSetShader(m_pPixelShaderSolid, nullptr, 0);
	m_pDeviceContext->DrawIndexed(m_nIndices, 0, 0);

	// ImGui rendering
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// ImGui 창 시작
	ImGui::Begin("Cube and Light");

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

	// 큐브 조정
	ImGui::Text("Cube Properties");
	ImGui::SliderFloat3("Cube Scale", &m_CubeScale.x, 0.1f, 2.0f);
	ImGui::SliderFloat3("Cube Rotation", &m_CubeRotation.x, 0.0f, 360.0f);

	// 카메라 조정
	ImGui::Text("Camera Position");
	ImGui::SliderFloat3("Camera Position", &m_CameraPos.x, -10.0f, 10.0f);

	ImGui::Checkbox("Enable Normal Map", &m_bNormalMapEnabled);
	ImGui::Checkbox("Enable Specular Map", &m_bSpecularMapEnabled);


	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Present our back buffer to our front buffer
	m_pSwapChain->Present(0, 0);
}

bool NormalMap::InitD3D()
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

void NormalMap::UninitD3D()
{
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pDeviceContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pRenderTargetView);
}

bool NormalMap::InitImGUI()
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

void NormalMap::UninitImGUI()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK NormalMap::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	return __super::WndProc(hWnd, message, wParam, lParam);
}

// 1. Render() 에서 파이프라인에 바인딩할 버텍스 버퍼및 버퍼 정보 준비
// 2. Render() 에서 파이프라인에 바인딩할 InputLayout 생성 	
// 3. Render() 에서 파이프라인에 바인딩할  버텍스 셰이더 생성
// 4. Render() 에서 파이프라인에 바인딩할 인덱스 버퍼 생성
// 5. Render() 에서 파이프라인에 바인딩할 픽셀 셰이더 생성
// 6. Render() 에서 파이프라인에 바인딩할 상수 버퍼 생성
bool NormalMap::InitScene()
{
	HRESULT hr = 0; // 결과값.
	ID3D10Blob* errorMessage = nullptr;	 // 에러 메시지를 저장할 버퍼.

	// 1. Render() 에서 파이프라인에 바인딩할 버텍스 버퍼및 버퍼 정보 준비
	// Local or Object or Model Space
	// Position, Tangent, Binormal, Normal, Texcoord
	Vertex vertices[] =
	{
		// Top face (y = +1)
		{ Vector3(-1.0f, 1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)},
		{ Vector3(1.0f, 1.0f, -1.0f),  Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),   Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),  Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f) },

		// Bottom face (y = -1)
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f),  Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f),   Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(-1.0f, -1.0f, 1.0f),  Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f) },

		// Left face (x = -1)
		{ Vector3(-1.0f, -1.0f, 1.0f),  Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f),  Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),   Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f) },

		// Right face (x = +1)
		{ Vector3(1.0f, -1.0f, 1.0f),   Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f),  Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f),   Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),    Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f) },

		// Front face (z = -1)
		{ Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, -1.0f),  Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, -1.0f),   Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, -1.0f),  Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 1.0f) },

		// Back face (z = +1)
		{ Vector3(-1.0f, -1.0f, 1.0f),  Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(1.0f, -1.0f, 1.0f),   Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 0.0f) },
		{ Vector3(1.0f, 1.0f, 1.0f),    Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(-1.0f, 1.0f, 1.0f),   Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 1.0f) },
	};

	// 버텍스 버퍼 생성.
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;
	HR_T(m_pDevice->CreateBuffer(&bd, &vbData, &m_pVertexBuffer));

	// 버텍스 버퍼 바인딩.
	m_VertexBufferStride = sizeof(Vertex);
	m_VertexBufferOffset = 0;


	// 2. Render() 에서 파이프라인에 바인딩할 InputLayout 생성 	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D10Blob* vertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_pInputLayout));

	// 3. Render() 에서 파이프라인에 바인딩할  버텍스 셰이더 생성
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader));

	SAFE_RELEASE(vertexShaderBuffer);

	// 4. Render() 에서 파이프라인에 바인딩할 인덱스 버퍼 생성
	WORD indices[] =
	{
		3,1,0, 2,1,3,
		6,4,5, 7,4,6,
		11,9,8, 10,9,11,
		14,12,13, 15,12,14,
		19,17,16, 18,17,19,
		22,20,21, 23,20,22
	};

	// 인덱스 개수 저장.
	m_nIndices = ARRAYSIZE(indices);

	bd = {};
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
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, &m_pConstantBuffer));


	// Load the Texture
	HR_T(CreateDDSTextureFromFile(m_pDevice, L"Bricks059_1K-JPG_Color.dds", nullptr, &m_pTextureRV));

	// Normal
	HR_T(CreateDDSTextureFromFile(m_pDevice, L"Bricks059_1K-JPG_NormalDX.dds", nullptr, &m_pNormalTextureRV));

	// Specular
	HR_T(CreateDDSTextureFromFile(m_pDevice, L"Bricks059_Specular.dds", nullptr, &m_pSpecularTextureRV));

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear));

	// TODO : Shader 수정하고 Render 수정하고 Vertex에 TexCoord 넣고

	// 초기값설정
	m_World = XMMatrixIdentity();

	XMVECTOR Eye = XMVectorSet(0.0f, 4.0f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_ClientWidth / (FLOAT)m_ClientHeight, 0.01f, 100.0f);
	return true;
}

void NormalMap::UninitScene()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pDepthStencilView);
}
