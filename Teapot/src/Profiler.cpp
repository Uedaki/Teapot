#include "Profiler.h"

#ifdef PROFILING

#include <chrono>
#include <thread>

#include "Log.h"

namespace
{
	Profiler profiler;
}


Profiler::FunctionProfiler::FunctionProfiler(const std::string &session, const std::string &name)
	: session(session)
{
	auto startPoint = std::chrono::high_resolution_clock::now();

	profile.categorie = "function";
	profile.name = name;
	profile.timeStart = std::chrono::time_point_cast<std::chrono::microseconds>(startPoint).time_since_epoch().count();
	profile.threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
}

Profiler::FunctionProfiler::~FunctionProfiler()
{
	auto endPoint = std::chrono::high_resolution_clock::now();
	long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();

	profile.duration = end - profile.timeStart;

	profiler.store(profile);
}

Profiler::Profiler()
{
	session.open("./profile.json", std::ios::out | std::ios::trunc);
	session << "{\"otherData\": {}, \"traceEvents\":[";

	startPoint = std::chrono::high_resolution_clock::now();
}

Profiler::~Profiler()
{
	session << "]}";
	session.close();
	
	LOG_MSG("Profile save");
}

void Profiler::store(const Profiler::ProfileResult &result)
{
	if (!profiler.isFirstProfile)
	{
		profiler.session << ",";
	}

	profiler.session << "{";
	profiler.session << "\"cat\":\"" << result.categorie << "\",";
	profiler.session << "\"dur\":" << result.duration << ",";
	profiler.session << "\"name\":\"" << result.name << "\",";
	profiler.session << "\"ph\":\"X\",";
	profiler.session << "\"pid\":0,";
	profiler.session << "\"tid\":" << result.threadId << ",";
	profiler.session << "\"ts\":" << result.timeStart << "";
	profiler.session << "}";

	profiler.session.flush();
	profiler.isFirstProfile = false;
}

void Profiler::startProfilingRegion(const std::string &session, const std::string &name)
{

}

void Profiler::endProfilingRegion(const std::string &session, const std::string &name)
{

}

std::chrono::time_point<std::chrono::steady_clock> Profiler::getSessionStartPoint() const
{
	return (startPoint);
}

#endif // PROFILING