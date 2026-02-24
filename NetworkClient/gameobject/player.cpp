#include	"player.h"	
#include    "../system/CDirectInput.h"
#include    "../system/camera.h" 

void player::init() {

	m_mesh = std::make_unique<CStaticMesh>();
	m_mesh->Load("assets/model/CheckedSphere.fbx", "assets/model");
	m_meshrenderer = std::make_unique<CStaticMeshRenderer>();
	m_meshrenderer->Init(*m_mesh);
	m_shader = std::make_unique<CShader>();
	m_shader->Create("shader/vertexLightingVS.hlsl", "shader/vertexLightingPS.hlsl");

	m_srt.pos = Vector3(0, 10, 0);
	m_srt.scale = Vector3(10, 10, 10);
	m_srt.rot = Vector3(0, 0, 0);

    m_RotationMatrix = DirectX::SimpleMath::Matrix::Identity;
}

void player::update(uint64_t dt) {
		move();
}

void player::draw(uint64_t dt) {
    using namespace DirectX::SimpleMath;
    Matrix matScale = Matrix::CreateScale(m_srt.scale);
    Matrix matTrans = Matrix::CreateTranslation(m_srt.pos);
    Matrix worldMtx = matScale * m_RotationMatrix * matTrans;	
    
    Renderer::SetWorldMatrix(&worldMtx);
	m_shader->SetGPU();
	m_meshrenderer->Draw();
}

void player::dispose() {
}

// 移動処理
void player::move() {
    // 入力方向を決定する
    Vector3 inputDir = Vector3(0, 0, 0);

    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W) ||
        CDirectInput::GetInstance().CheckKeyBuffer(DIK_UPARROW)) {
        inputDir.z += 1.0f;
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S) ||
        CDirectInput::GetInstance().CheckKeyBuffer(DIK_DOWNARROW)) {
        inputDir.z -= 1.0f;
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_D) ||
        CDirectInput::GetInstance().CheckKeyBuffer(DIK_RIGHTARROW)) {
        inputDir.x += 1.0f;
    }
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_A) ||
        CDirectInput::GetInstance().CheckKeyBuffer(DIK_LEFTARROW)) {
        inputDir.x -= 1.0f;
    }

    // X軸の加減速処理
    if (inputDir.x != 0.0f) {
        MOVE_SPEED.x += inputDir.x * MOVE_ACCELARATION;
    }
    else {
        if (fabs(MOVE_SPEED.x) < MOVE_ACCELARATION) {
            MOVE_SPEED.x = 0.0f;
        }
        else {
            if (MOVE_SPEED.x > 0) MOVE_SPEED.x -= MOVE_ACCELARATION;
            else                  MOVE_SPEED.x += MOVE_ACCELARATION;
        }
    }

    // Z軸加減速処理
    if (inputDir.z != 0.0f) {
        MOVE_SPEED.z += inputDir.z * MOVE_ACCELARATION;
    }
    else {
        if (fabs(MOVE_SPEED.z) < MOVE_ACCELARATION) {
            MOVE_SPEED.z = 0.0f;
        }
        else {
            if (MOVE_SPEED.z > 0) MOVE_SPEED.z -= MOVE_ACCELARATION;
            else                  MOVE_SPEED.z += MOVE_ACCELARATION;
        }
    }

    // 速度制限
    if (MOVE_SPEED.x > MAX_SPEED)  MOVE_SPEED.x = MAX_SPEED;
    if (MOVE_SPEED.x < -MAX_SPEED) MOVE_SPEED.x = -MAX_SPEED;
    if (MOVE_SPEED.z > MAX_SPEED)  MOVE_SPEED.z = MAX_SPEED;
    if (MOVE_SPEED.z < -MAX_SPEED) MOVE_SPEED.z = -MAX_SPEED;

    // 座標への反映
    Vector3 pos = getSRT().pos;
    pos.x += MOVE_SPEED.x;
    pos.z += MOVE_SPEED.z;
    setSRTPosition(pos);


    if (MOVE_SPEED.Length() > 0.001f) {
        // 移動ベクトルと回転軸の計算
        using namespace DirectX::SimpleMath;
        Vector3 moveDir = MOVE_SPEED;

        // 進行方向と上(0,1,0)の外積で、回転軸を出す
        Vector3 up(0.0f, 1.0f, 0.0f);
        Vector3 rotateAxis = up.Cross(moveDir);
        rotateAxis.Normalize(); 

        // 回転角度の計算 
        float radius = m_srt.scale.x;
        float angle = moveDir.Length() / radius;

        // 今回の回転行列を作成 
        Matrix deltaRot = Matrix::CreateFromAxisAngle(rotateAxis, angle);

        // 今の回転に行列を合成
        m_RotationMatrix *= deltaRot;
    }

    // 進行方向の回転角
    if (MOVE_SPEED.Length() > 0.001f) {
        yaw = std::atan2(MOVE_SPEED.x, MOVE_SPEED.z);
    }
}