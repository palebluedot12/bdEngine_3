#pragma once

/*
	���� ������Ʈ�� ����� ������Ʈ ������ �и��ϱ� ���� �߻� Ŭ����
	���ӿ�����Ʈ�� ������Ʈ�� �������� �̷������.
	������Ʈ�� ������ GameObject�� CreateComponent<T> �� ���� �̷������.
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

