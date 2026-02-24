#pragma once
#include "gameobject.h"
#include <memory> // スマートポインタ用

// 必要なラッパークラスをインクルード
#include "../system/CVertexBuffer.h"
#include "../system/CIndexBuffer.h"
#include "../system/CShader.h"
#include "../system/CMaterial.h"
#include "../system/CTexture.h"

class message : public gameobject
{
public:
    void update(uint64_t delta) override;
    void draw(uint64_t delta) override;
    void init() override;
    void dispose() override; // ← これの中身は空でよくなる！

    // 画像を読み込む関数
    void LoadTexture(const char* filename);

private:
    // スマートポインタで管理！
    // new しても delete を書かなくて済むようになる
    std::unique_ptr<CVertexBuffer<VERTEX_3D>> m_VertexBuffer;
    std::unique_ptr<CIndexBuffer> m_IndexBuffer;
    std::unique_ptr<CShader>      m_Shader;
    std::unique_ptr<CMaterial>    m_Material;
    std::unique_ptr<CTexture>     m_Texture;
};