#include "message.h"
#include "../system/renderer.h"

void message::update(uint64_t delta) {
}

void message::draw(uint64_t delta) {
    if (!m_VertexBuffer) return;

    // ワールド行列セット
    Matrix4x4 mtx = m_srt.GetMatrix();
    Renderer::SetWorldMatrix(&mtx);

    // 各種リソースをGPUにセット
    m_Shader->SetGPU();
    m_Material->SetGPU();
    if (m_Texture) m_Texture->SetGPU();
    m_VertexBuffer->SetGPU();
    m_IndexBuffer->SetGPU();

    // 描画実行
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(6, 0, 0);
}

void message::init() {
    m_VertexBuffer = std::make_unique<CVertexBuffer<VERTEX_3D>>();
    m_IndexBuffer = std::make_unique<CIndexBuffer>();
    m_Shader = std::make_unique<CShader>();
    m_Material = std::make_unique<CMaterial>();
    m_Texture = std::make_unique<CTexture>();

    // 頂点データの作成
    std::vector<VERTEX_3D> vertices;
    VERTEX_3D v;
    float w = 0.5f;
    float d = 0.5f;
    float y_offset = 0.1f;

    // 左奥 (UV: 0,0)
    v.Position = Vector3(-w, y_offset, d);
    v.Normal = Vector3(0, 1, 0); 
    v.TexCoord = Vector2(0.0f, 0.0f);
    v.Diffuse = Color(1, 1, 1, 1);
    vertices.push_back(v);

    // 右奥 (UV: 1,0)
    v.Position = Vector3(w, y_offset, d);
    v.Normal = Vector3(0, 1, 0);
    v.TexCoord = Vector2(1.0f, 0.0f);
    v.Diffuse = Color(1, 1, 1, 1);
    vertices.push_back(v);

    // 左手前 (UV: 0,1)
    v.Position = Vector3(-w, y_offset, -d);
    v.Normal = Vector3(0, 1, 0);
    v.TexCoord = Vector2(0.0f, 1.0f);
    v.Diffuse = Color(1, 1, 1, 1);
    vertices.push_back(v);

    // 右手前 (UV: 1,1)
    v.Position = Vector3(w, y_offset, -d);
    v.Normal = Vector3(0, 1, 0);
    v.TexCoord = Vector2(1.0f, 1.0f);
    v.Diffuse = Color(1, 1, 1, 1);
    vertices.push_back(v);

    m_VertexBuffer->Create(vertices);

    // インデックスデータの作成
    std::vector<uint32_t> indices = { 0, 1, 2, 2, 1, 3 };
    m_IndexBuffer->Create(indices);

    // シェーダー作成
    m_Shader->Create("shader/vertexLightingVS.hlsl", "shader/vertexLightingPS.hlsl");

    // マテリアル初期設定
    MATERIAL mtrl{};
    mtrl.Diffuse = Color(1, 1, 1, 1);
    mtrl.TextureEnable = TRUE;
    m_Material->Create(mtrl);

    // 初期トランスフォーム
    m_srt.pos = { 0, 0, 0 };
    m_srt.scale = { 20, 1, 20 };
}

void message::LoadTexture(const char* filename) {
    if (m_Texture) {
        m_Texture->Load(filename);
    }
}

void message::dispose() {
}