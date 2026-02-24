#pragma once

#include	"commontypes.h"
#include	"renderer.h"
#include	"../application.h"
#include	"CPolar3D.h"
#include    "CDirectInput.h"
#include    <algorithm>

class Camera {
protected:
	Vector3	m_position = Vector3(0.0f, 0.0f, 0.0f);		// カメラ位置
	Vector3	m_lookat{};									// 注視点
	Vector3 m_centerPos{};                             // 回転の中心座標
	Matrix4x4	m_viewmtx{};							// ビュー行列
	Matrix4x4   m_projmtx{};							// プロジェクション行列

	Vector3 m_BEVPosition = Vector3(0.0f, 600.0f, -150.0f); // 俯瞰カメラの位置（Bird Eye View）

public:
	Camera() = default;

	void Init();
	void Dispose();
	void Update();
	void Draw();
	void SetPosition(const Vector3& position) { m_position = position; }
	void SetLookat(const Vector3& position) { m_lookat = position; }
	void SetCenterPosition(const Vector3& centerPos) { m_centerPos = centerPos; }

	Matrix4x4 GetViewMatrix() const { return m_viewmtx; }
	Matrix4x4 GetProjMatrix() const { return m_projmtx; }

	Vector3 GetPosition() const { return m_position; }
	Vector3 GetLookat() const { return m_lookat; }

	Vector3 GetBEVPosition() const { return m_BEVPosition; }
};

// / 自由視点カメラ
class FreeCamera : public Camera {
private:
	float m_elevation = -90.0f * PI / 180.0f;		// 仰角
	float m_azimuth = PI / 2.0f;						// 方位角	
	float m_radius = 100.0f;								// 半径	

	Vector3 m_up = { 0.0f, 1.0f, 0.0f };			// 上方向ベクトル
public:
	void SetElevation(float elevation) {
		m_elevation = elevation;
	}

	void SetAzimuth(float azimuth) {
		m_azimuth = azimuth;
	}

	void SetRadius(float radius) 
{
		m_radius = radius;
	}

	void CalcCameraPosition() 
	{
		// 仰角と方位角からカメラの位置を計算
		CPolor3D polar(m_radius, m_elevation, m_azimuth);
		// 極座標からデカルト座標に変換
		m_position = polar.ToCartesian();

		CPolor3D polarup(m_radius, (m_elevation + (90.0f * PI / 180.0f)), m_azimuth);
		m_up = polarup.ToCartesian();
	}

	//void CalcCameraPositionTranslate()
	//{
	//	// 注視点を中心にカメラの位置を計算
	//	CPolor3D polar(m_radius, m_elevation, m_azimuth);
	//	// 極座標からデカルト座標に変換
	//	m_position = polar.ToCartesian();
	//	m_position += m_lookat;	// 注視点を中心にカメラの位置を設定
	//	CPolor3D polarup(m_radius, (m_elevation + (90.0f * PI / 180.0f)), m_azimuth);
	//	m_up = polarup.ToCartesian();
	//}

	void Draw() {
		// ビュー変換後列作成
		m_viewmtx =
			DirectX::XMMatrixLookAtLH(
				m_position,
				m_lookat,
				m_up);				// 左手系にした　20230511 by suzuki.tomoki

		// DIRECTXTKのメソッドは右手系　20230511 by suzuki.tomoki
		// 右手系にすると３角形頂点が反時計回りになるので描画されなくなるので注意
		// このコードは確認テストのために残す
		//	m_ViewMatrix = m_ViewMatrix.CreateLookAt(m_Position, m_Target, up);					

		Renderer::SetViewMatrix(&m_viewmtx);

		//プロジェクション行列の生成
		constexpr float fieldOfView = DirectX::XMConvertToRadians(45.0f);    // 視野角

		float aspectRatio = static_cast<float>(Application::GetWidth()) / static_cast<float>(Application::GetHeight());	// アスペクト比	
		float nearPlane = 1.0f;			// ニアクリップ
		float farPlane = 3000.0f;		// ファークリップ

		//プロジェクション行列の生成
		Matrix4x4 projectionMatrix;
		projectionMatrix =
			DirectX::XMMatrixPerspectiveFovLH(
				fieldOfView,
				aspectRatio,
				nearPlane,
				farPlane);	// 左手系にした　20230511 by suzuki.tomoki

		// DIRECTXTKのメソッドは右手系　20230511 by suzuki.tomoki
		// 右手系にすると３角形頂点が反時計回りになるので描画されなくなるので注意
		// このコードは確認テストのために残す
		//	projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(fieldOfView, aspectRatio, nearPlane, farPlane);

		Renderer::SetProjectionMatrix(&projectionMatrix);

	}

	/**
	 * @brief 指定された中心座標を中心にカメラ位置を計算
	 * @param centerPos 回転の中心となる座標
	 */
	void CalcCameraPositionAroundCenter()
	{
		// 仰角と方位角からカメラの位置を計算
		CPolor3D polar(m_radius, m_elevation, m_azimuth);
		// 極座標からデカルト座標に変換
		m_position = polar.ToCartesian();

		// 中心座標を加算
		m_position += m_centerPos;

		// Upベクトルの計算 (ここでは簡略化のためワールドY軸基準)
		m_up = Vector3(0.0f, 1.0f, 0.0f);
	}

	void Update() {
		// 制御変数
		static float radius = 800.0f;
		static float elevation_control = 0.0f;
		static float azimuth = PI / 2.0f;

		// --- マウス操作定数 ---
		const float CAMERA_SENSITIVITY = 0.005f;
		const float SCROLL_SENSITIVITY = 0.5f;

		// 仰角制限範囲
		const float ELEVATION_MIN = -PI / 2.0f;
		const float ELEVATION_MAX = PI / 2.0f;

		float mouse_wheel = (float)CDirectInput::GetInstance().GetMouseScrollZ();

		if (CDirectInput::GetInstance().GetMouseRButtonCheck())
		{
			// マウスのデルタ（移動量）を取得
			float mouse_delta_x = (float)CDirectInput::GetInstance().GetMouseDeltaX();
			float mouse_delta_y = (float)CDirectInput::GetInstance().GetMouseDeltaY();

			// 左右回転
			azimuth -= mouse_delta_x * CAMERA_SENSITIVITY;

			// 上下回転
			elevation_control += mouse_delta_y * CAMERA_SENSITIVITY;

			// 仰角制限
			elevation_control = std::clamp(elevation_control, ELEVATION_MIN + 0.01f, ELEVATION_MAX - 0.01f);
		}

		// マウスホイールで距離 (Radius) 変更
		if (std::abs(mouse_wheel) > 0.0f)
		{
			radius -= mouse_wheel * SCROLL_SENSITIVITY;
			radius = std::clamp(radius, 1.0f, 1500.0f);
		}

		// --- カメラへの値の設定と変換 ---
		// 仰角の基準シフト
		float elevation = elevation_control - PI / 2.0f;

		// カメラ設定適用
		SetRadius(radius);
		SetElevation(elevation);
		SetAzimuth(azimuth);

		// プレイヤー座標を中心としたカメラ位置を計算
		CalcCameraPositionAroundCenter();
	}
};