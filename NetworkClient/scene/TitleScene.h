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
#include "../gameobject/TextureUI.h"

class TitleScene :
	public IScene
{
public:
	/// @brief コピーコンストラクタは使用不可
	TitleScene(const TitleScene&) = delete;
	/// @brief 代入演算子も使用不可
	TitleScene& operator=(const TitleScene&) = delete;

	explicit TitleScene();

	void update(uint64_t deltatime) override;
	void draw(uint64_t deltatime) override;
	void init() override;
	void dispose() override;

	void debugROGO();

private:
	std::unique_ptr<Camera> m_camera;
	std::unique_ptr<field> m_field;
	std::unique_ptr<TextureUI> m_ROGO;
};

REGISTER_CLASS(TitleScene)