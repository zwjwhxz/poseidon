#include "../precompiled.hpp"
#include "utilities.hpp"
#include <signal.h>
#include "log.hpp"
#include "exception.hpp"
#include "singletons/main_config.hpp"
#include "singletons/timer_daemon.hpp"
#include "singletons/mysql_daemon.hpp"
#include "singletons/epoll_daemon.hpp"
#include "singletons/player_servlet_manager.hpp"
#include "singletons/http_servlet_manager.hpp"
#include "singletons/websocket_servlet_manager.hpp"
#include "singletons/system_http_server.hpp"
#include "singletons/job_dispatcher.hpp"
#include "singletons/module_manager.hpp"
#include "singletons/event_listener_manager.hpp"
#include "singletons/profile_manager.hpp"
#include "profiler.hpp"
using namespace Poseidon;

namespace {

void sigTermProc(int){
	LOG_WARNING("Received SIGTERM, will now exit...");
	JobDispatcher::quitModal();
}

void sigIntProc(int){
	static const unsigned long long SAFETY_TIMER_EXPIRES = 300 * 1000000;
	static unsigned long long s_safetyTimer = 0;

	// 系统启动的时候这个时间是从 0 开始的，如果这时候按下 Ctrl+C 就会立即终止。
	// 因此将计时器的起点设为该区间以外。
	const unsigned long long now = getMonoClock() + SAFETY_TIMER_EXPIRES + 1;
	if(s_safetyTimer + SAFETY_TIMER_EXPIRES < now){
		s_safetyTimer = now + 5 * 1000000;
	}
	if(now < s_safetyTimer){
		LOG_WARNING("Received SIGINT, trying to exit gracefully... "
			"If I don't terminate in 5 seconds, press ^C again.");
		::raise(SIGTERM);
	} else {
		LOG_FATAL("Received SIGINT, will now terminate abnormally...");
		::raise(SIGKILL);
	}
}

template<typename T>
struct RaiiSingletonRunner : boost::noncopyable {
	RaiiSingletonRunner(){
		T::start();
	}
	~RaiiSingletonRunner(){
		T::stop();
	}
};

#define START(x_)	const RaiiSingletonRunner<x_> UNIQUE_ID

void run(){
	PROFILE_ME;

	START(MySqlDaemon);
	START(TimerDaemon);
	START(EpollDaemon);

	START(PlayerServletManager);
	START(HttpServletManager);
	START(WebSocketServletManager);
	START(EventListenerManager);
	START(SystemHttpServer);

	START(ModuleManager);

	LOG_INFO("Waiting for all MySQL operations to complete...");
	MySqlDaemon::waitForAllAsyncOperations();

	LOG_INFO("Entering modal loop...");
	JobDispatcher::doModal();
}

}

int main(int argc, char **argv){
	Logger::setThreadTag(Logger::TAG_PRIMARY);
	LOG_INFO("-------------------------- Starting up -------------------------");

	START(ProfileManager);

	LOG_INFO("Setting up signal handlers...");
	::signal(SIGINT, sigIntProc);
	::signal(SIGTERM, sigTermProc);

	try {
		MainConfig::setRunPath((1 < argc) ? argv[1] : "/var/poseidon");
		MainConfig::reload();

		unsigned logLevel = Logger::LV_INFO;
		MainConfig::get(logLevel, "log_level");
		LOG_INFO("Setting log level to ", logLevel, "...");
		Logger::setLevel(logLevel);

		run();

		LOG_INFO("------------- Server has been shut down gracefully -------------");
		return EXIT_SUCCESS;
	} catch(std::exception &e){
		LOG_ERROR("std::exception thrown in main(): what = ", e.what());
	} catch(...){
		LOG_ERROR("Unknown exception thrown in main().");
	}

	LOG_WARNING("----------------- Server has exited abnormally -----------------");
	return EXIT_FAILURE;
}
