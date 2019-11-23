#pragma once

#include <stdexcept>

#define EXCEPTION(msg) throw teapot::Exception(__FUNCSIG__, msg)
#define CRITICAL_EXCEPTION(msg) throw teapot::CriticalException(__FUNCSIG__, msg)

namespace teapot
{
	class Exception : public std::exception
	{
	protected:
		std::string fullMsg;

	public:
		Exception(const char *function, const char *msg)
		{
			fullMsg = "Exception throwed with message \"";
			fullMsg += msg;
			fullMsg += "\" in \"";
			fullMsg += function;
			fullMsg += "\"";
		}

		virtual const char *what() const noexcept
		{
			return (fullMsg.c_str());
		}
	};

	class CriticalException : public teapot::Exception
	{
	public:
		CriticalException(const char *function, const char *msg)
			: Exception(function, msg)
		{}
	};
}