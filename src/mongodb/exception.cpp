// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2018, LH_Mouse. All wrongs reserved.

#include "../precompiled.hpp"
#include "exception.hpp"
#include "../log.hpp"

namespace Poseidon {
namespace Mongo_db {

Exception::Exception(const char *file, std::size_t line, const char *func, Shared_nts database, unsigned long code, Shared_nts message)
	: Basic_exception(file, line, func, STD_MOVE(message))
	, m_database(STD_MOVE(database)), m_code(code)
{
	LOG_POSEIDON_ERROR("Mongo_db::Exception: database = ", get_database(), ", code = ", get_code(), ", what = ", what());
}
Exception::~Exception() NOEXCEPT {
	//
}

}
}
