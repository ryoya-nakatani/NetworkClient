#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h> 
#include <windows.h>

#include <array>
#include <memory>

#include "../system/camera.h"
#include "../system/IScene.h"
#include "../system/C3DShape.h"
#include "../system/transform.h"
#include "../system/SceneClassFactory.h"

#include "../gameobject/field.h"

class MatchingScene :
    public IScene
{
public:
	/// @brief コピーコンストラクタは使用不可
	MatchingScene(const MatchingScene&) = delete;
	/// @brief 代入演算子も使用不可
	MatchingScene& operator=(const MatchingScene&) = delete;

	explicit MatchingScene();

	void update(uint64_t deltatime) override;
	void draw(uint64_t deltatime) override;
	void init() override;
	void dispose() override;

private:
	std::unique_ptr<Camera> m_camera;
	std::unique_ptr<field> m_field;
};

REGISTER_CLASS(MatchingScene)