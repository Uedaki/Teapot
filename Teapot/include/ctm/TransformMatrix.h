#pragma once

#include <glm/glm.hpp>

namespace ctm
{
	struct TransformMatrix
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};
}