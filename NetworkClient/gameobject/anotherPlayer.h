#pragma once

#include	<memory>
#include	"gameobject.h"
#include	"../system/CStaticMesh.h"
#include	"../system/CStaticMeshRenderer.h"
#include	"../system/CShader.h"

class AnotherPlayer : public gameobject {
public:
	void update(uint64_t delta) override;
	void draw(uint64_t delta) override;
	void init() override;
	void dispose() override;

	void move();		// 移動処理

	float GetYaw() { return yaw; }
	void SetMyID(uint32_t id) { my_id = id; }
	auto GetMyID() { return my_id; }

	// 動きのパラメータ
	Vector3 MOVE_SPEED = { 0.0f,0.0f,0.0f };			// 移動速度
	const float MOVE_ACCELARATION = 1.0f;	// 加速度
	const float MAX_SPEED = 5.0f;					// 最高速度

private:
	std::unique_ptr<CStaticMesh>		m_mesh;
	std::unique_ptr<CStaticMeshRenderer>m_meshrenderer;
	std::unique_ptr<CShader>m_shader;
	DirectX::SimpleMath::Matrix m_RotationMatrix;
	float yaw = 0.0f;

	uint32_t my_id = 0;
};