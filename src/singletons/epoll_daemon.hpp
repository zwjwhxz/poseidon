// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2018, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_SINGLETONS_EPOLL_DAEMON_HPP_
#define POSEIDON_SINGLETONS_EPOLL_DAEMON_HPP_

#include "../cxx_ver.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include "../ip_port.hpp"

namespace Poseidon {

class Socket_base;

class Epoll_daemon {
public:
	struct Snapshot_element {
		Ip_port remote_info;
		Ip_port local_info;
		boost::uint64_t creation_time;
		bool listening;
		bool readable;
		bool writeable;
	};

private:
	Epoll_daemon();

public:
	static void start();
	static void stop();

	static void add_socket(const boost::shared_ptr<Socket_base> &socket, bool take_ownership = false);
	static bool mark_socket_writeable(const Socket_base *ptr) NOEXCEPT;

	static void snapshot(boost::container::vector<Snapshot_element> &ret);
};

}

#endif
