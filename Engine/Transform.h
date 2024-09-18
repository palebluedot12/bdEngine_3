//#pragma once
//#include "Component.h"
//
//class Transform : public Component
//{
//public:
//	Transform();
//	virtual ~Transform();
//
//public:
//	Transform* m_pParentScene = nullptr;	// 부모 Scene 컴포넌트	
//	D2D_VECTOR_2F		m_RelativeScale = { 1,1 };	// 상대 크기
//	float				m_RelativeRotation = 0; // 상대 회전
//	D2D_VECTOR_2F 		m_RelativeLocation = { 0,0 }; // 상대 위치
//	D2D1_MATRIX_3X2_F	m_RelativeTransform; // 상대 복합 변환
//	D2D1_MATRIX_3X2_F	m_WorldTransform;    // 부모까지 반영된 최종 변환
//
//	// RelativeTransform과 	m_WorldTransform을 계산한다.
//	void UpdateTransform();
//	void SetParentScene(Transform* pParentScene) { m_pParentScene = pParentScene; }
//	D2D1_VECTOR_2F GetWorldLocation()
//	{
//		D2D1_VECTOR_2F out;
//		out.x = m_WorldTransform._31;
//		out.y = m_WorldTransform._32;
//		return out;
//	}
//
//	virtual void Update() override;
//	void DrawDebugScene(ID2D1RenderTarget* pRenderTarget);
//
//	void SetRelativeScale(float x, float y) { m_RelativeScale.x = x; m_RelativeScale.y = y; }
//
//	void SetRelativeRotation(float Rotation);
//	void AddRelativeRotation(float Rotation);
//	float GetRelativeRotation() { return m_RelativeRotation; }
//
//	void SetRelativeLocation(const D2D1_VECTOR_2F& Location);
//	void SetRelativeLocation(float x, float y);
//	void AddRelativeLocation(float x, float y);
//	const D2D1_VECTOR_2F& GetRelativeLocation() { return m_RelativeLocation; }
//
//};
//
