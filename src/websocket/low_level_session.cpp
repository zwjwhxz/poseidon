// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2018, LH_Mouse. All wrongs reserved.

#include "../precompiled.hpp"
#include "low_level_session.hpp"
#include "exception.hpp"
#include "../http/low_level_session.hpp"
#include "../log.hpp"
#include "../profiler.hpp"

namespace Poseidon {
namespace Web_socket {

Low_level_session::Low_level_session(const boost::shared_ptr<Http::Low_level_session> &parent)
	: Http::Upgraded_session_base(parent), Reader(true), Writer()
{
	//
}
Low_level_session::~Low_level_session(){
	//
}

void Low_level_session::on_connect(){
	//
}
void Low_level_session::on_read_hup(){
	//
}
void Low_level_session::on_close(int /*err_code*/){
	//
}
void Low_level_session::on_receive(Stream_buffer data){
	PROFILE_ME;

	Reader::put_encoded_data(STD_MOVE(data));
}

void Low_level_session::on_data_message_header(Op_code opcode){
	PROFILE_ME;

	on_low_level_message_header(opcode);
}
void Low_level_session::on_data_message_payload(boost::uint64_t whole_offset, Stream_buffer payload){
	PROFILE_ME;

	on_low_level_message_payload(whole_offset, STD_MOVE(payload));
}
bool Low_level_session::on_data_message_end(boost::uint64_t whole_size){
	PROFILE_ME;

	return on_low_level_message_end(whole_size);
}

bool Low_level_session::on_control_message(Op_code opcode, Stream_buffer payload){
	PROFILE_ME;

	return on_low_level_control_message(opcode, STD_MOVE(payload));
}

long Low_level_session::on_encoded_data_avail(Stream_buffer encoded){
	PROFILE_ME;

	return Upgraded_session_base::send(STD_MOVE(encoded));
}

bool Low_level_session::send(Op_code opcode, Stream_buffer payload, bool masked){
	PROFILE_ME;

	return Writer::put_message(opcode, masked, STD_MOVE(payload));
}

bool Low_level_session::shutdown(Status_code status_code, const char *reason) NOEXCEPT
try {
	PROFILE_ME;

	if(has_been_shutdown_write()){
		return false;
	}
	Writer::put_close_message(status_code, false, Stream_buffer(reason));
	shutdown_read();
	return shutdown_write();
} catch(std::exception &e){
	LOG_POSEIDON_ERROR("std::exception thrown: what = ", e.what());
	force_shutdown();
	return false;
} catch(...){
	LOG_POSEIDON_ERROR("Unknown exception thrown.");
	force_shutdown();
	return false;
}

}
}
