#include	"scenemanager.h"
#include	"SceneClassFactory.h"
#include	"DebugUI.h"

// “o˜^‚³‚ê‚Ä‚¢‚éƒV[ƒ“‚ğ‘S‚Ä”jŠü‚·‚é
void SceneManager::Dispose() 
{
	// “o˜^‚³‚ê‚Ä‚¢‚é‚·‚×‚ÄƒV[ƒ“‚ÌI—¹ˆ—
	for (auto& s : m_scenes) 
	{
		s.second->dispose();
	}

	m_scenes.clear();
	m_currentSceneName.clear();
}

void SceneManager::SetCurrentScene(std::string currentscenename) 
{
	DebugUI::ClearDebugFunctions();

	if (!m_currentSceneName.empty()) {
		if (m_scenes.find(m_currentSceneName) != m_scenes.end()) {
			m_scenes[m_currentSceneName]->dispose();
			m_scenes.erase(m_currentSceneName);
		}
	}

	m_currentSceneName = currentscenename;
	auto obj = SceneClassFactory::getInstance().create(currentscenename);
	obj->init();
	m_scenes[m_currentSceneName] = std::move(obj);
}

void SceneManager::Init()
{
}

void SceneManager::Draw(uint64_t deltatime)
{

	// Œ»İ‚ÌƒV[ƒ“‚ğ•`‰æ
	m_scenes[m_currentSceneName]->draw(deltatime);
}

void SceneManager::Update(uint64_t deltatime)
{
	// Œ»İ‚ÌƒV[ƒ“‚ğXV
	m_scenes[m_currentSceneName]->update(deltatime);
}