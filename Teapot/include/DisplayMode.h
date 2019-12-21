#pragma once

#include <stdint.h>

namespace teapot
{
	enum class DisplayMode : int8_t
	{
		NONE = -1,
		FACE = 0,
		EDGE = 1,
		VERTEX = 2
	};
}