// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2018, LH_Mouse. All wrongs reserved.

#include "../precompiled.hpp"
#include "exception.hpp"
#include "../log.hpp"

namespace Poseidon {
namespace Http {

namespace {
	const Optional_map g_empty_headers;
}

const Optional_map &empty_headers() NOEXCEPT {
	return g_empty_headers;
}

Exception::Exception(const char *file, std::size_t line, const char *func, Status_code status_code, Optional_map headers)
	: Basic_exception(file, line, func, Shared_nts::view(get_status_code_desc(status_code).desc_short))
	, m_status_code(status_code), m_headers(headers.empty() ? boost::shared_ptr<Optional_map>()
	                                                        : boost::make_shared<Optional_map>(STD_MOVE(headers)))
{
	LOG_POSEIDON(Logger::special_major | Logger::level_info, "Http::Exception: status_code = ", get_status_code(), ", what = ", what());
}
Exception::~Exception() NOEXCEPT {
	//
}

}
}
