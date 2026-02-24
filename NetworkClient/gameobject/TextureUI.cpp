#include "TextureUI.h"
#include "../system/renderer.h" 

void TextureUI::init()
{
    // 頂点データの作成（XY平面の正方形）
    std::vector<VERTEX_3D> vertices;
    VERTEX_3D v;

    // 左上
    v.Position = Vector3(0.0f, 0.0f, 0.0f);
    v.TexCoord = Vector2(0.0f, 0.0f);
    v.Normal = Vector3(0, 0, -1); // 手前向き
    v.Diffuse = Color(1, 1, 1, 1);
    vertices.push_back(v);

    // 右上
    v.Position = Vector3(1.0f, 0.0f, 0.0f);
    v.TexCoord = Vector2(1.0f, 0.0f);
    v.Normal = Vector3(0, 0, -1);
    v.Diffuse = Color(1, 1, 1, 1);
    vertices.push_back(v);

    // 左下
    v.Position = Vector3(0.0f, 1.0f, 0.0f);
    v.TexCoord = Vector2(0.0f, 1.0f);
    v.Normal = Vector3(0, 0, -1);
    v.Diffuse = Color(1, 1, 1, 1);
    vertices.push_back(v);

    // 右下
    v.Position = Vector3(1.0f, 1.0f, 0.0f);
    v.TexCoord = Vector2(1.0f, 1.0f);
    v.Normal = Vector3(0, 0, -1);
    v.Diffuse = Color(1, 1, 1, 1);
    vertices.push_back(v);

    m_VertexBuffer.Create(vertices);

    // インデックスデータの作成（四角形1枚分）
    std::vector<uint32_t> indices = { 0, 1, 2, 2, 1, 3 };
    m_IndexBuffer.Create(indices);

    // シェーダーとマテリアル
    m_Shader.Create("shader/vertexLightingVS.hlsl", "shader/vertexLightingPS.hlsl");

    MATERIAL mtrl{};
    mtrl.Diffuse = Color(1, 1, 1, 1);
    mtrl.TextureEnable = TRUE;
    m_Material.Create(mtrl);
}

void TextureUI::Load(const char* filename)
{
    m_Texture.Load(filename);
}

void TextureUI::draw(uint64_t delta)
{
    // Zバッファ無効化
    Renderer::SetDepthEnable(false);

    // 2D用カメラセット
    Renderer::SetWorldViewProjection2D();

    // ワールド行列で配置
    Matrix4x4 world, scale, trans;
    scale = Matrix4x4::CreateScale(m_size.x, m_size.y, 1.0f);
    trans = Matrix4x4::CreateTranslation(m_pos.x, m_pos.y, 0.0f);
    world = scale * trans;
    Renderer::SetWorldMatrix(&world);

    // 描画実行
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_Shader.SetGPU();
    m_VertexBuffer.SetGPU();
    m_IndexBuffer.SetGPU();
    m_Material.SetGPU();
    m_Texture.SetGPU();

    context->DrawIndexed(6, 0, 0);

    Renderer::SetDepthEnable(true);
}

void TextureUI::dispose()
{
}