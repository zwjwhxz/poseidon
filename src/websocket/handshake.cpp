// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2018, LH_Mouse. All wrongs reserved.

#include "../precompiled.hpp"
#include "handshake.hpp"
#include "../log.hpp"
#include "../sha1.hpp"
#include "../random.hpp"
#include "../profiler.hpp"
#include "../base64.hpp"

namespace Poseidon {
namespace Web_socket {

Http::Response_headers make_handshake_response(const Http::Request_headers &request){
	PROFILE_ME;

	Http::Response_headers response = { };
	response.version = 10001;
	{
		if(request.version < 10001){
			LOG_POSEIDON(Logger::special_major | Logger::level_debug, "HTTP 1.1 is required to use Web_socket");
			response.status_code = Http::status_version_not_supported;
			goto _done;
		}
		if(request.verb != Http::verb_get){
			LOG_POSEIDON(Logger::special_major | Logger::level_debug, "Must use GET verb to use Web_socket");
			response.status_code = Http::status_method_not_allowed;
			goto _done;
		}
		const AUTO_REF(websocket_version, request.headers.get("Sec-Web_socket-Version"));
		char *endptr;
		const AUTO(version_num, std::strtol(websocket_version.c_str(), &endptr, 10));
		if(*endptr){
			LOG_POSEIDON(Logger::special_major | Logger::level_debug, "Unrecognized HTTP header Sec-Web_socket-Version: ", websocket_version);
			response.status_code = Http::status_bad_request;
			goto _done;
		}
		if((version_num < 0) || (version_num < 13)){
			LOG_POSEIDON(Logger::special_major | Logger::level_debug, "Unsupported Sec-Web_socket-Version: ", websocket_version);
			response.status_code = Http::status_bad_request;
			goto _done;
		}
		const AUTO_REF(sec_websocket_key, request.headers.get("Sec-Web_socket-Key"));
		if(sec_websocket_key.empty()){
			LOG_POSEIDON(Logger::special_major | Logger::level_debug, "No Sec-Web_socket-Key specified.");
			response.status_code = Http::status_bad_request;
			goto _done;
		}
		Sha1_ostream sha1_os;
		sha1_os <<sec_websocket_key <<"258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		const AUTO(sha1, sha1_os.finalize());
		Base64_encoder enc;
		enc.put(sha1.data(), sha1.size());
		AUTO(sec_websocket_accept, enc.finalize().dump_string());
		response.headers.set(sslit("Upgrade"), "websocket");
		response.headers.set(sslit("Connection"), "Upgrade");
		response.headers.set(sslit("Sec-Web_socket-Accept"), STD_MOVE(sec_websocket_accept));
		response.status_code = Http::status_switching_protocols;
	}
_done:
	response.reason = Http::get_status_code_desc(response.status_code).desc_short;
	return response;
}

std::pair<Http::Request_headers, std::string> make_handshake_request(std::string uri, Optional_map get_params, std::string host){
	PROFILE_ME;

	Http::Request_headers request = { };
	request.verb       = Http::verb_get;
	request.uri        = STD_MOVE(uri);
	request.version    = 10001;
	request.get_params = STD_MOVE(get_params);
	request.headers.set(sslit("Host"), STD_MOVE(host));
	request.headers.set(sslit("Upgrade"), "websocket");
	request.headers.set(sslit("Connection"), "Keep-Alive");
	request.headers.set(sslit("Sec-Web_socket-Version"), "13");
	request.headers.set(sslit("Pragma"), "no-cache");
	request.headers.set(sslit("Cache-Control"), "no-cache");
	boost::uint32_t key[4];
	for(unsigned i = 0; i < 4; ++i){
		key[i] = random_uint32();
	}
	Base64_encoder enc;
	enc.put(key, sizeof(key));
	AUTO(sec_websocket_key, enc.finalize().dump_string());
	request.headers.set(sslit("Sec-Web_socket-Key"), sec_websocket_key);
	return std::make_pair(STD_MOVE_IDN(request), STD_MOVE_IDN(sec_websocket_key));
}
bool check_handshake_response(const Http::Response_headers &response, const std::string &sec_websocket_key){
	PROFILE_ME;

	if(response.version < 10001){
		LOG_POSEIDON(Logger::special_major | Logger::level_debug, "HTTP 1.1 is required to use Web_socket");
		return false;
	}
	if(response.status_code != Http::status_switching_protocols){
		LOG_POSEIDON(Logger::special_major | Logger::level_debug, "Bad HTTP status code: ", response.status_code);
		return false;
	}
	const AUTO_REF(upgrade, response.headers.get("Upgrade"));
	if(upgrade.empty() || (::strcasecmp(upgrade.c_str(), "websocket") != 0)){
		LOG_POSEIDON(Logger::special_major | Logger::level_debug, "Invalid Upgrade header: ", upgrade);
		return false;
	}
	const AUTO_REF(connection, response.headers.get("Connection"));
	if(connection.empty() || (::strcasecmp(connection.c_str(), "Upgrade") != 0)){
		LOG_POSEIDON(Logger::special_major | Logger::level_debug, "Invalid Connection header: ", connection);
		return false;
	}
	const AUTO_REF(sec_websocket_accept, response.headers.get("Sec-Web_socket-Accept"));
	if(sec_websocket_accept.empty()){
		LOG_POSEIDON(Logger::special_major | Logger::level_debug, "No Sec-Web_socket-Accept specified.");
		return false;
	}
	Sha1_ostream sha1_os;
	sha1_os <<sec_websocket_key <<"258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	const AUTO(sha1, sha1_os.finalize());
	Base64_encoder enc;
	enc.put(sha1.data(), sha1.size());
	AUTO(sec_websocket_accept_expecting, enc.finalize().dump_string());
	if(sec_websocket_accept != sec_websocket_accept_expecting){
		LOG_POSEIDON(Logger::special_major | Logger::level_debug, "Bad Sec-Web_socket-Accept: got ", sec_websocket_accept, ", expecting ", sec_websocket_accept_expecting);
		return false;
	}
	return true;
}

}
}
