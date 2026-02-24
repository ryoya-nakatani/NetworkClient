#include "MatchingScene.h"

#include "../system/meshmanager.h"
#include "../system/CStaticMesh.h"
#include "../system/CStaticMeshRenderer.h"
#include "../system/CDirectInput.h"
#include "../system/scenemanager.h"

MatchingScene::MatchingScene()
{
}

void MatchingScene::update(uint64_t deltatime) {
	// カメラ
	m_camera->Update();
}

void MatchingScene::draw(uint64_t deltatime) {
	m_camera->Draw();
	m_field->draw(deltatime);

	// シーン遷移
	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_2)) {
		SceneManager::SetCurrentScene("WalkThroughScene");
	}
}

void MatchingScene::init() {
	// カメラの初期化
	m_camera = std::make_unique<Camera>();
	m_camera->Init();

	// 光源計算なしシェーダー
	std::unique_ptr<CShader> shader = std::make_unique<CShader>();
	shader->Create("shader/vertexLightingVS.hlsl", "shader/vertexLightingPS.hlsl");
	MeshManager::RegisterShader<CShader>("unlightshader", std::move(shader));

	// フィールド初期化
	m_field = std::make_unique<field>();
	m_field->init();
}

void MatchingScene::dispose() {

}
