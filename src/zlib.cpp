// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2018, LH_Mouse. All wrongs reserved.

#include "precompiled.hpp"
#include "zlib.hpp"
#include "profiler.hpp"
#include "exception.hpp"

namespace Poseidon {

Deflator::Deflator(bool gzip, int level){
	m_stream.zalloc = NULLPTR;
	m_stream.zfree = NULLPTR;
	m_stream.opaque = NULLPTR;
	m_stream.next_in = NULLPTR;
	m_stream.avail_in = 0;
	int err_code = ::deflateInit2(&m_stream, level, Z_DEFLATED, 15 + gzip * 16, 9, Z_DEFAULT_STRATEGY);
	DEBUG_THROW_UNLESS(err_code >= 0, Exception, sslit("::deflateInit2()"));
}
Deflator::~Deflator(){
	int err_code = ::deflateEnd(&m_stream);
	if(err_code < 0){
		LOG_POSEIDON_WARNING("::deflateEnd() error: err_code = ", err_code);
	}
}

void Deflator::clear(){
	PROFILE_ME;

	int err_code = ::deflateReset(&m_stream);
	if(err_code < 0){
		LOG_POSEIDON_FATAL("::deflateReset() error: err_code = ", err_code);
		std::abort();
	}
	m_buffer.clear();
}
void Deflator::put(const void *data, std::size_t size){
	PROFILE_ME;

	const AUTO(begin, static_cast<const unsigned char *>(data));
	m_stream.next_in = begin;
	m_stream.avail_in = 0;
	int err_code;
	for(;;){
		if(m_stream.avail_in == 0){
			std::size_t remaining = static_cast<std::size_t>(begin + size - m_stream.next_in);
			if(remaining > UINT_MAX){
				remaining = UINT_MAX;
			}
			m_stream.avail_in = static_cast<unsigned>(remaining);
		}
		if(m_stream.avail_in == 0){
			break;
		}
		unsigned char temp[4096];
		m_stream.next_out = temp;
		m_stream.avail_out = sizeof(temp);
		err_code = ::deflate(&m_stream, Z_NO_FLUSH);
		DEBUG_THROW_UNLESS(err_code >= 0, Exception, sslit("::deflate()"));
		m_buffer.put(temp, static_cast<unsigned>(m_stream.next_out - temp));
		DEBUG_THROW_ASSERT(err_code == 0);
	}
}
void Deflator::put(const Stream_buffer &buffer){
	PROFILE_ME;

	const void *data;
	std::size_t size;
	Stream_buffer::Enumeration_cookie cookie;
	while(buffer.enumerate_chunk(&data, &size, cookie)){
		put(data, size);
	}
}
void Deflator::flush(){
	PROFILE_ME;

	m_stream.next_in = NULLPTR;
	m_stream.avail_in = 0;
	int err_code;
	for(;;){
		unsigned char temp[4096];
		m_stream.next_out = temp;
		m_stream.avail_out = sizeof(temp);
		err_code = ::deflate(&m_stream, Z_SYNC_FLUSH);
		if(err_code == Z_BUF_ERROR){
			break;
		}
		DEBUG_THROW_UNLESS(err_code >= 0, Exception, sslit("::deflate()"));
		m_buffer.put(temp, static_cast<unsigned>(m_stream.next_out - temp));
		DEBUG_THROW_ASSERT(err_code == 0);
	}
}
Stream_buffer Deflator::finalize(){
	PROFILE_ME;

	m_stream.next_in = NULLPTR;
	m_stream.avail_in = 0;
	int err_code;
	for(;;){
		unsigned char temp[4096];
		m_stream.next_out = temp;
		m_stream.avail_out = sizeof(temp);
		err_code = ::deflate(&m_stream, Z_FINISH);
		DEBUG_THROW_UNLESS(err_code >= 0, Exception, sslit("::deflate()"));
		m_buffer.put(temp, static_cast<unsigned>(m_stream.next_out - temp));
		if(err_code == Z_STREAM_END){
			break;
		}
		DEBUG_THROW_ASSERT(err_code == 0);
	}
	Stream_buffer ret;
	ret.swap(m_buffer);
	clear();
	return ret;
}

Inflator::Inflator(bool gzip){
	m_stream.zalloc = NULLPTR;
	m_stream.zfree = NULLPTR;
	m_stream.opaque = NULLPTR;
	m_stream.next_in = NULLPTR;
	m_stream.avail_in = 0;
	int err_code = ::inflateInit2(&m_stream, 15 + gzip * 16);
	DEBUG_THROW_UNLESS(err_code >= 0, Exception, sslit("::deflateInit2()"));
}
Inflator::~Inflator(){
	int err_code = ::inflateEnd(&m_stream);
	if(err_code < 0){
		LOG_POSEIDON_WARNING("::inflateEnd() error: err_code = ", err_code);
	}
}

void Inflator::clear(){
	PROFILE_ME;

	int err_code = ::inflateReset(&m_stream);
	if(err_code < 0){
		LOG_POSEIDON_FATAL("::inflateReset() error: err_code = ", err_code);
		std::abort();
	}
	m_buffer.clear();
}
void Inflator::put(const void *data, std::size_t size){
	PROFILE_ME;

	const AUTO(begin, static_cast<const unsigned char *>(data));
	m_stream.next_in = begin;
	m_stream.avail_in = 0;
	int err_code;
	for(;;){
		if(m_stream.avail_in == 0){
			std::size_t remaining = static_cast<std::size_t>(begin + size - m_stream.next_in);
			if(remaining > UINT_MAX){
				remaining = UINT_MAX;
			}
			m_stream.avail_in = static_cast<unsigned>(remaining);
		}
		if(m_stream.avail_in == 0){
			break;
		}
		unsigned char temp[4096];
		m_stream.next_out = temp;
		m_stream.avail_out = sizeof(temp);
		err_code = ::inflate(&m_stream, Z_NO_FLUSH);
		DEBUG_THROW_UNLESS(err_code >= 0, Exception, sslit("::inflate()"));
		m_buffer.put(temp, static_cast<unsigned>(m_stream.next_out - temp));
		if(err_code == Z_STREAM_END){
			break;
		}
		DEBUG_THROW_ASSERT(err_code == 0);
	}
}
void Inflator::put(const Stream_buffer &buffer){
	PROFILE_ME;

	const void *data;
	std::size_t size;
	Stream_buffer::Enumeration_cookie cookie;
	while(buffer.enumerate_chunk(&data, &size, cookie)){
		put(data, size);
	}
}
void Inflator::flush(){
	PROFILE_ME;

	m_stream.next_in = NULLPTR;
	m_stream.avail_in = 0;
	int err_code;
	for(;;){
		unsigned char temp[4096];
		m_stream.next_out = temp;
		m_stream.avail_out = sizeof(temp);
		err_code = ::inflate(&m_stream, Z_SYNC_FLUSH);
		if(err_code == Z_BUF_ERROR){
			break;
		}
		DEBUG_THROW_UNLESS(err_code >= 0, Exception, sslit("::inflate()"));
		m_buffer.put(temp, static_cast<unsigned>(m_stream.next_out - temp));
		DEBUG_THROW_ASSERT(err_code == 0);
	}
}
Stream_buffer Inflator::finalize(){
	PROFILE_ME;

	m_stream.next_in = NULLPTR;
	m_stream.avail_in = 0;
	int err_code;
	for(;;){
		unsigned char temp[4096];
		m_stream.next_out = temp;
		m_stream.avail_out = sizeof(temp);
		err_code = ::inflate(&m_stream, Z_FINISH);
		DEBUG_THROW_UNLESS(err_code >= 0, Exception, sslit("::inflate()"));
		m_buffer.put(temp, static_cast<unsigned>(m_stream.next_out - temp));
		if(err_code == Z_STREAM_END){
			break;
		}
		DEBUG_THROW_ASSERT(err_code == 0);
	}
	Stream_buffer ret;
	ret.swap(m_buffer);
	clear();
	return ret;
}

}
