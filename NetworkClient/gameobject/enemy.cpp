#include	"enemy.h"	
#include    "../system/CDirectInput.h"
#include	"../system/meshmanager.h"

#include	"../scene/WalkThroughScene.h"

void enemy::init() 
{
	m_mesh = MeshManager::getMesh<CStaticMesh>("car001.x");
	m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("car001.x");
	m_shader = MeshManager::getShader<CShader>("unlightshader");
}

void enemy::update(uint64_t dt) {

	player* p;

	p = ((WalkThroughScene*)(m_ownerscene))->getplayer();	// ƒvƒŒƒCƒ„‚ÌŽæ“¾

	// ƒvƒŒƒCƒ„‚ÌÀ•W‚ðŽæ“¾
	SRT srt= p->getSRT();
	Vector3 playerpos = srt.pos;

	// atan2‚ðŽg—p‚µ‚ÄŠp“x‚ð‹‚ß‚é
	m_destrot.y = atan2f(-(playerpos.x - m_srt.pos.x), -(playerpos.z - m_srt.pos.z));

	// Œ»Ý‚ÌŒü‚«‚Æ‚Ì·•ª‚ðŒvŽZ‚·‚é
	float fDiffRotY = m_destrot.y - m_srt.rot.y;

	// •â³i|‚P‚W‚O`‚P‚W‚O‚Ì”ÍˆÍj
	if (fDiffRotY > PI)
	{
		fDiffRotY -= PI * 2.0f;
	}
	if (fDiffRotY < -PI)
	{
		fDiffRotY += PI * 2.0f;
	}

	// ‰ñ“]Šp“xŒvŽZ
	m_srt.rot.y += fDiffRotY * RATE_ROTATE_MODEL;
	if (m_srt.rot.y > PI)
	{
		m_srt.rot.y -= PI * 2.0f;
	}
	if (m_srt.rot.y < -PI)
	{
		m_srt.rot.y += PI * 2.0f;
	}

	Matrix4x4 mtx = Matrix4x4::CreateRotationY(m_srt.rot.y);	// –Ú•WŠp“x‚ðŒ»Ý‚ÌŠp“x‚ÉXV
	Vector3 forward = mtx.Forward();	// ‘O•ûƒxƒNƒgƒ‹‚ðŽæ“¾

	m_move = forward * m_speed;			// ‘O•ûƒxƒNƒgƒ‹‚ÉƒXƒs[ƒh‚ðŠ|‚¯‚ÄˆÚ“®—Ê‚ðŒvŽZ

	/// ˆÊ’uˆÚ“®
	m_srt.pos += m_move;

	// ˆÚ“®—Ê‚ÉŠµ«‚ð‚©‚¯‚é(Œ¸‘¬—¦)
	m_move += -m_move * RATE_MOVE_MODEL;

}

void enemy::draw(uint64_t dt) {


	Matrix4x4 mtx = m_srt.GetMatrix();

	Renderer::SetWorldMatrix(&mtx);

	m_shader->SetGPU();
	m_meshrenderer->Draw();

}

void enemy::dispose() {

}
