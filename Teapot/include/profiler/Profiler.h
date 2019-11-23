#pragma once

#ifdef PROFILING

#include <chrono>
#include <fstream>
#include <map>
#include <string>

#define PROFILE_SCOPE(session, name) Profiler::FunctionProfiler __hidden_scope_profiler__(session, name)
#define PROFILE_FUNCTION(session) Profiler::FunctionProfiler __hidden_scope_profiler__(session, __FUNCSIG__)

class Profiler
{
	struct ProfileResult
	{
		std::string categorie;
		long long timeStart;
		long long duration;
		std::string name;
		int pid;
		size_t threadId;
	};

public:
	class FunctionProfiler
	{
		friend class Profiler;

		std::string session;
		ProfileResult profile;

	public:
		FunctionProfiler(const std::string &session, const std::string &name);
		~FunctionProfiler();
	};

	static void store(const ProfileResult &result);

	static void startProfilingRegion(const std::string &session, const std::string &name);
	static void endProfilingRegion(const std::string &session, const std::string &name);

	Profiler();
	~Profiler();
	std::chrono::time_point<std::chrono::steady_clock> getSessionStartPoint() const;

private:
	bool isFirstProfile = true;

	std::ofstream session;

	std::chrono::time_point<std::chrono::steady_clock> startPoint;
};

#else

#define PROFILE_SCOPE(session, name)
#define PROFILE_FUNCTION(session)

#endif // !PROFILING
