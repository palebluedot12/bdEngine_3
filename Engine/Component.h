#pragma once

/*
	게임 오브젝트의 기능을 컴포넌트 단위로 분리하기 위한 추상 클래스
	게임오브젝트는 컴포넌트의 조합으로 이루어진다.
	컴포넌트의 생성은 GameObject의 CreateComponent<T> 를 통해 이루어진다.
*/

class GameObject;
class Component
{
public:
	Component();
	virtual ~Component();

public:
	GameObject* GetOwner() { return m_Owner; }
	void SetOwner(GameObject* pOwner) { m_Owner = pOwner; }

	virtual void Init();
	virtual void Update();
	virtual void Render();

protected:
	GameObject* m_Owner = nullptr;

};

