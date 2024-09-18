//#include "pch.h"
//#include "Transform.h"
//
//Transform::Transform()
//{
//	m_RelativeTransform = m_WorldTransform = D2D1::Matrix3x2F::Identity();
//}
//
//Transform::~Transform()
//{
//
//}
//
//void Transform::UpdateTransform()
//{
//	m_RelativeTransform = D2D1::Matrix3x2F::Scale(D2D1::SizeF(m_RelativeScale.x, m_RelativeScale.y)) *
//		D2D1::Matrix3x2F::Rotation(m_RelativeRotation) *
//		D2D1::Matrix3x2F::Translation(m_RelativeLocation.x, m_RelativeLocation.y);
//
//	if (m_pParentScene != nullptr)
//		m_WorldTransform = m_RelativeTransform * m_pParentScene->m_WorldTransform;
//	else
//		m_WorldTransform = m_RelativeTransform;
//}
//
//void Transform::Update()
//{
//	UpdateTransform();
//}
//
//
//void Transform::DrawDebugScene(ID2D1RenderTarget* pRenderTarget)
//{
//#if _DEBUG	
//	pRenderTarget->SetTransform(m_WorldTransform);
//	D2DRenderer::Get()->DrawRectangle(Vector2D(1, 1), Vector2D(30, 30), Color::RED);
//	//pRenderTarget->DrawRectangle(D2D1::RectF(-2, -2, 2, 2), Color::RED);
//#endif // _DEBUG
//}
//
//void Transform::SetRelativeRotation(float Rotation)
//{
//	m_RelativeRotation = Rotation;
//	m_RelativeRotation = fmodf(m_RelativeRotation, 360.0f);
//}
//
//void Transform::AddRelativeRotation(float Rotation)
//{
//	m_RelativeRotation += Rotation;
//	m_RelativeRotation = fmodf(m_RelativeRotation, 360.0f);
//}
//
//
//void Transform::SetRelativeLocation(const D2D1_VECTOR_2F& Location)
//{
//	m_RelativeLocation = Location;
//}
//
//void Transform::SetRelativeLocation(float x, float y)
//{
//	m_RelativeLocation.x = x;
//	m_RelativeLocation.y = y;
//}
//
//void Transform::AddRelativeLocation(float x, float y)
//{
//	m_RelativeLocation = { m_RelativeLocation.x + x,m_RelativeLocation.y + y };
//}
