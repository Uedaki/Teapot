#pragma once

#include <Imgui.h>

#include "gui/Widget.h"

struct ImVec2;

namespace teapot
{
	namespace gui
	{
		class SceneEditorWidget : public Widget
		{
		public:
			void draw() override;

		private:
			void handleCameraMovement(ImVec2 size);
			void handleSelection(ImVec2 size);

			int mouseButtonIndex = -1;
			ImVec2 mouseLastPosition;
		};
	}
}