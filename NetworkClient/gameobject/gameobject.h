#pragma once
#include <cstdint>
#include "../system/Network/ClientSystem.h"
#include "../system/transform.h"

class gameobject {
public:
	gameobject() = default;
	virtual ~gameobject() = default;
	virtual void update(uint64_t delta) = 0;
	virtual void draw(uint64_t delta) = 0;
	virtual void init() = 0;
	virtual void dispose() = 0;

	SRT getSRT() {
		return m_srt;
	}
	void	setSRT(const SRT& srt) {
		m_srt = srt;
	}

	void setSRTPosition(const Vector3& pos) {
		m_srt.pos = pos;
	}
	void setSRTRotation(const Vector3& rot) {
		m_srt.rot = rot;
	}

protected:
	SRT		m_srt{};
};