#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class CapsuleVolume : CollisionVolume
	{
	public:
		CapsuleVolume(const float h, const float r) {
			type = VolumeType::Capsule;
			height = h;
			radius = r;
		}
		~CapsuleVolume() {

		}

		float GetHeight() { return height; }
		float GetRadius() { return radius; }

	protected:
		float radius;
		float height;
	};
}
