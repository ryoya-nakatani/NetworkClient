#pragma once
#include "gameobject.h"
#include "../system/CVertexBuffer.h"
#include "../system/CIndexBuffer.h"
#include "../system/CShader.h"
#include "../system/CMaterial.h"
#include "../system/CTexture.h"

class TextureUI : public gameobject
{
public:
    void update(uint64_t delta) override {} 
    void draw(uint64_t delta) override;
    void init() override;
    void dispose() override;

    // 画像ファイルを指定してロードする
    void Load(const char* filename);

    // サイズを指定する（ピクセル単位）
    void SetSize(float w, float h) { m_size = Vector2(w, h); }

    // 位置を指定する（画面左上が原点）
    void SetPosition(float x, float y) { m_pos = Vector2(x, y); }
    void SetPosition(Vector2 pos) { m_pos = pos; }

    // 位置を取得する
    Vector2 GetPosition() { return m_pos; }

private:
    CVertexBuffer<VERTEX_3D> m_VertexBuffer;
    CIndexBuffer m_IndexBuffer;
    CShader m_Shader;
    CMaterial m_Material;
    CTexture m_Texture;

    Vector2 m_pos = { 0, 0 };
    Vector2 m_size = { 100, 100 };
};