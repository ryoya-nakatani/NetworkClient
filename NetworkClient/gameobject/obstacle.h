#pragma once

#include	<memory>
#include	"gameobject.h"
#include	"../system/CStaticMesh.h"
#include	"../system/CStaticMeshRenderer.h"
#include	"../system/CShader.h"
#include	"../system/IScene.h"

class obstacle : public gameobject {

public:
	obstacle(IScene* currentscene)
		: m_meshrenderer(nullptr),
		m_ownerscene(currentscene) {
	}

	void update(uint64_t delta) override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

private:
	CStaticMesh*			m_mesh{};
	CStaticMeshRenderer*	m_meshrenderer{};
	CShader*				m_shader{};

	// オーナーSCENE
	IScene* m_ownerscene = nullptr;
};