#pragma once

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
			void handleSelection(ImVec2 size);
		};
	}
}