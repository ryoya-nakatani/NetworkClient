#include "TitleScene.h"

#include "../system/meshmanager.h"
#include "../system/CStaticMesh.h"
#include "../system/CStaticMeshRenderer.h"
#include "../system/CDirectInput.h"
#include "../system/scenemanager.h"
#include "../system/debugui.h"

// デバッグ用IMGUI
void TitleScene::debugROGO()
{
	Vector2 ROGOPosition = m_ROGO->GetPosition();

	ImGui::Begin("debug ROGO");

	ImGui::SliderFloat2("ROGO Position", &ROGOPosition.x, -1000, 1000);
	m_ROGO->SetPosition(ROGOPosition);

	ImGui::End();
}

TitleScene::TitleScene()
{
}

void TitleScene::update(uint64_t deltatime) {
	// カメラ
	m_camera->Update();

	// シーン遷移
	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_RETURN)) {
		SceneManager::SetCurrentScene("WalkThroughScene");
	}
}

void TitleScene::draw(uint64_t deltatime) {
	m_camera->Draw();
	m_field->draw(deltatime);
	m_ROGO->draw(deltatime);
}

void TitleScene::init() {
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

	// タイトルロゴ初期化
	m_ROGO = std::make_unique<TextureUI>();
	m_ROGO->init();
	m_ROGO->Load("assets/texture/LINK_ROGO.png");
	m_ROGO->SetPosition(350, -50);
	m_ROGO->SetSize(600, 600);

#ifdef _DEBUG
	DebugUI::RedistDebugFunction([this]() {
		debugROGO();
		});
#endif // _DEBUG
}

void TitleScene::dispose() {

}
