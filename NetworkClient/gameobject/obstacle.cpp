#include	"obstacle.h"	
#include    "../system/CDirectInput.h"
#include	"../system/meshmanager.h"

#include	"../scene/WalkThroughScene.h"

void obstacle::init() 
{
	m_mesh = MeshManager::getMesh<CStaticMesh>("obstaclebox");
	m_shader = MeshManager::getShader<CShader>("unlightshader");
	m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("obstaclebox");
}

void obstacle::update(uint64_t dt) {


}

void obstacle::draw(uint64_t dt) {


	Matrix4x4 mtx = m_srt.GetMatrix();

	Renderer::SetWorldMatrix(&mtx);

	m_shader->SetGPU();
	m_meshrenderer->Draw();

}

void obstacle::dispose() {

}
