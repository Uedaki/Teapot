#pragma once

#include "gui/Widget.h"
#include "vulkan/Mesh.h"

namespace teapot
{
	namespace gui
	{
		class AttributeEditorWidget : public Widget
		{
		public:
			void draw() override;

		private:
			vk::Mesh *currentObject;

			uint32_t mode = 0;

			bool transform = true;
			glm::vec3 location = glm::vec3(0, 0, 0);
			glm::vec3 rotation = glm::vec3(0, 0, 0);
			glm::vec3 scale = glm::vec3(1, 1, 1);
		};
	}
}