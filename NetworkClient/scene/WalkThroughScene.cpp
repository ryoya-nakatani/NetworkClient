#include "../system/Network/ClientSystem.h"
#include <string>
#include <array>
#include <chrono>

#include "WalkThroughScene.h"

#include "../system/debugui.h"
#include "../system/AimOrientation.h"
#include "../system/SphereDrawer.h"
#include "../system/ConeDrawer.h"
#include "../system/LineDrawer.h"
#include "../system/TriangleDrawer.h"
#include "../system/meshmanager.h"
#include "../system/RandomEngine.h"
#include "../system/scenemanager.h"
#include "../system/Network/MessageManager.h"
#include "../system/Network/NetworkStats.h"
#include "../system/Network/DataBase.h"

#define DEBUG

using namespace std::literals::chrono_literals;
namespace {
}

void WalkThroughScene::debugCamera()
{
	Vector3 cameraPosition = m_camera->GetBEVPosition();

	ImGui::Begin("debug Camera");

	ImGui::SliderFloat3("Camera Position", &cameraPosition.x, -1000, 1000);
	m_camera->SetBEVPosition(cameraPosition);

	ImGui::End();
}

/**
 * @brief コンストラクタ
 */
WalkThroughScene::WalkThroughScene()
{
}

/**
 * @brief シーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void WalkThroughScene::update(uint64_t deltatime)
{
	switch (state)
	{
	case IDOL:
		// プレイヤ
		m_player->update(deltatime);

		if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_1)) {
			state = SET_MESSAGE;
			isSetMessage = true;
		}
		break;
	case SET_MESSAGE:
		if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_RETURN)) {
			ClientSystem::GetInstance().GetTCPManager()->SendSetMessage(m_player->getSRT().pos.x, m_player->getSRT().pos.y, m_player->getSRT().pos.z, m_player->GetYaw());
			state = IDOL;
			isSetMessage = false;
		}
		if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_BACKSPACE)) {
			state = IDOL;
			isSetMessage = false;
		}
		break;
	default:
		break;
	}

	
	// カメラ
	Vector3 playerPos = m_player->getSRT().pos;
	Vector3 BEVPos = m_camera->GetBEVPosition();
	m_camera->SetPosition(Vector3(playerPos.x + BEVPos.x, playerPos.y + BEVPos.y, playerPos.z + BEVPos.z));
	m_camera->SetLookat(playerPos);
	m_camera->Update();

	// イベントチェック
	if(NetworkStats::GetInstance().GetConnectState()==ConnectState::Disconnection) 
		ClientSystem::GetInstance().GetTCPManager()->Connect();
	ClientSystem::GetInstance().GetTCPManager()->CheckTcpEvents();

	// メッセージオブジェクト更新
	SyncMessages();

	NetworkStats::GetInstance().Update(deltatime);

	// シーン遷移
	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_ESCAPE)) {
		SceneManager::SetCurrentScene("TitleScene");
	}
}

/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void WalkThroughScene::draw(uint64_t deltatime)
{
	m_camera->Draw();

	// 3軸カラー
	Color axiscol[3] = {
		Color(1, 0, 0, 1),
		Color(0, 1, 0, 1),
		Color(0, 1, 1, 1)
	};

	// ワールド軸を描画
	SetLineWidth(1.0f);					// 太さを設定
	for (int axisno = 0; axisno < 3; axisno++)
	{
		Matrix4x4 rotmtx = Matrix4x4::Identity;
		m_segments[axisno]->Draw(rotmtx, axiscol[axisno]);
	}

	// フィールド
//	Renderer::SetFillMode(D3D11_FILL_WIREFRAME);
	m_field->draw(deltatime);
//	Renderer::SetFillMode(D3D11_FILL_SOLID);

	// プレイヤ
	m_player->draw(deltatime);

	// メッセージ
	for (auto& msgObj : m_messageObjects) {
		if (msgObj->m_message) {
			msgObj->m_message->draw(deltatime);
		}
	}

	// メッセージ設置ウィンドウ
	if(isSetMessage)	m_messageWindow->draw(deltatime);

	// 平行光源の方向を示す矢印を描画 
	LIGHT l = Renderer::GetLight();
	Vector3 dir = Vector3(-l.Direction.x, -l.Direction.y, -l.Direction.z);

	AimOrientation aimorien(dir);
	aimorien.VisualizeDirection(
		Vector3(0, 100, 0), 20, 1, Color(1, 1, 0, 1), 2, Color(1, 0, 0, 1)
	);

	Vector3 sp;
	SRT srt = m_player->getSRT();
	sp = srt.pos;
	sp.y -= 500.0f;
	SetLineWidth(3.0f);
	LineDrawerDraw(1000, sp, Vector3(0, 1, 0), Color(1, 1, 0, 1));

	int sqno = m_field->GetSquareNo(srt.pos);

	std::array<field::Face, 2> retfaces;
	std::array<Vector3, 3> vertices1;
	std::array<Vector3, 3> vertices2;
	if (sqno != -1) {
		m_field->GetFace(srt.pos, retfaces);
		m_field->GetFaceVertex(sqno * 2, vertices1);
		m_field->GetFaceVertex(sqno * 2 + 1, vertices2);

		TriangleDrawerDraw(vertices1, Color(1, 0, 0, 1));
		TriangleDrawerDraw(vertices2, Color(1, 1, 0, 1));
	}
}

/**
 * @brief シーンの初期化処理
 */
void WalkThroughScene::init()
{
	// カメラ(3D)の初期化
	m_camera = std::make_unique<Camera>();
	m_camera->Init();

	// ローカル軸表示用線分の初期化
	m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));

	m_playersegment[0] = std::make_unique<Segment>(Vector3(0, -100, 0), Vector3(0, 100, 0));

	// 光源計算なしシェーダー
	std::unique_ptr<CShader> shader = std::make_unique<CShader>();
	shader->Create("shader/vertexLightingVS.hlsl", "shader/vertexLightingPS.hlsl");
	MeshManager::RegisterShader<CShader>("unlightshader", std::move(shader));

	// メッシュデータ読み込み（敵用）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/car001.x", "assets/model/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("car001.x", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("car001.x", std::move(srenderer));
	}

	// メッシュデータ読み込み（障害物用）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/obj/box.obj", "assets/model/obj/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("obstaclebox", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("obstaclebox", std::move(srenderer));
	}


	// フィールド初期化
	m_field = std::make_unique<field>();
	m_field->init();

	// プレイヤ
	m_player = std::make_unique<player>();
	m_player->init();

	// メッセージシンク用
	ClientSystem::GetInstance().GetTCPManager()->CheckTcpEvents();

	// メッセージ
	auto& messages = DataBase::GetInstance().GetMessages();
	for (const auto& msg : messages) {
		auto msgObj = std::make_unique<MessageObject>();
		msgObj->m_message = std::make_unique<message>();
		msgObj->m_message->init();
		msgObj->m_message->LoadTexture("assets/texture/message.png");
		msgObj->m_message->setSRTPosition(Vector3(msg.x, msg.y, msg.z));
		msgObj->m_message->setSRTRotation(Vector3(0.0f, msg.rotY, 0.0f));
		msgObj->isAlive = true;
		m_messageObjects.push_back(std::move(msgObj));
	}

	// メッセージ設置ウィンドウ初期化
	m_messageWindow = std::make_unique<TextureUI>();
	m_messageWindow->init();
	m_messageWindow->Load("assets/texture/SetMessageWindow.png");
	m_messageWindow->SetPosition(384, 216);
	m_messageWindow->SetSize(512, 288);

#ifdef DEBUG
	// デバッグ Camera
	DebugUI::RedistDebugFunction([this]() {
		debugCamera();
		});

	DebugUI::RedistDebugFunction([this]() {
		NetworkStats::GetInstance().DrawDebugUI();
		});

	DebugUI::RedistDebugFunction([this]() {
		NetworkStats::GetInstance().DrawMessageListUI();
		});

#endif // DEBUG

	TriangleDrawerInit();
}

/**
 * @brief シーンの終了処理
 */
void WalkThroughScene::dispose()
{
}

// メッセージリストからオブジェクトへのシンク
void WalkThroughScene::SyncMessages() {
	const auto& serverDataList = DataBase::GetInstance().GetMessages();

	// 判定の許容範囲（SendSetMessageの重複チェックと同じくらいにする）
	const float THRESHOLD = 5.0f;
	const float THRESHOLD_SQ = THRESHOLD * THRESHOLD;

	for (auto& obj : m_messageObjects) {
		obj->isAlive = false;
	}

	// サーバーのデータと照合
	for (const auto& data : serverDataList) {
		bool found = false;

		// 既存リストの中から、このデータと同じ場所にあるやつを探す
		for (auto& obj : m_messageObjects) {
			float dx = obj->m_message->getSRT().pos.x - data.x;
			float dy = obj->m_message->getSRT().pos.y - data.y;
			float dz = obj->m_message->getSRT().pos.z - data.z;
			float distSq = dx * dx + dy * dy + dz * dz;

			// 同じ場所にあるなら
			if (distSq < THRESHOLD_SQ) {
				obj->isAlive = true;
				found = true;
				break;
			}
		}

		// 新規メッセージ
		if (!found) {
			auto newObj = std::make_unique<MessageObject>();
			newObj->m_message = std::make_unique<message>();
			newObj->m_message->init();
			newObj->m_message->LoadTexture("assets/texture/message.png");
			newObj->m_message->setSRTPosition(Vector3(data.x, data.y, data.z));
			newObj->m_message->setSRTRotation(Vector3(0.0f, data.rotY, 0.0f));
			newObj->isAlive = true;

			// リストに追加
			m_messageObjects.push_back(std::move(newObj));
		}
	}

	// サーバーから消えたやつを削除
	for (auto it = m_messageObjects.begin(); it != m_messageObjects.end(); ) {
		if (!(*it)->isAlive) {
			it = m_messageObjects.erase(it);
		}
		else {
			++it;
		}
	}
}

void WalkThroughScene::SyncAnotherPlayers()
{
	auto& clients = DataBase::GetInstance().GetClients();
	auto& playersInfo = DataBase::GetInstance().GetPlayersInfo();
	int myID = ClientSystem::GetInstance().GetTCPManager()->GetMyID();

	// 他プレイヤーの数をカウント
	size_t otherPlayersCount = std::count_if(clients.begin(), clients.end(), [&](const auto& c) {
		return c.id != myID;
		});

	// オブジェクトの生成、削除
	if (otherPlayersCount > m_anotherPlayers.size()) {
		size_t diff = otherPlayersCount - m_anotherPlayers.size();
		for (size_t i = 0; i < diff; i++) {
			m_anotherPlayers.push_back(std::make_unique<AnotherPlayer>());
			m_anotherPlayers.back()->init();
		}
	}
	else if (otherPlayersCount < m_anotherPlayers.size()) {
		m_anotherPlayers.resize(otherPlayersCount);
	}

	// データの同期
	int targetIdx = 0;	// m_anotherPlayers 用のインデックス
	for (int i = 0; i < clients.size(); i++) {
		if (clients[i].id == myID) continue;	// 自分自身はスキップ
		if (targetIdx >= m_anotherPlayers.size()) break;	// 念のためのチェック

		m_anotherPlayers[targetIdx]->SetMyID(clients[i].id);
		m_anotherPlayers[targetIdx]->setSRTPosition(Vector3(
			playersInfo[m_anotherPlayers[targetIdx]->GetMyID()].x,
			playersInfo[m_anotherPlayers[targetIdx]->GetMyID()].y,
			playersInfo[m_anotherPlayers[targetIdx]->GetMyID()].z));

		targetIdx++;
	}

	DataBase::GetInstance().SetClientListChanged(false);
}