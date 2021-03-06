// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2018, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_OPTIONAL_MAP_HPP_
#define POSEIDON_OPTIONAL_MAP_HPP_

#include "cxx_ver.hpp"
#include <boost/container/map.hpp>
#include <stdexcept>
#include <iosfwd>
#include "shared_nts.hpp"

namespace Poseidon {

extern const std::string &empty_string() NOEXCEPT;

class Optional_map {
public:
	typedef boost::container::multimap<Shared_nts, std::string> base_container;

	typedef base_container::value_type        value_type;
	typedef base_container::const_reference   const_reference;
	typedef base_container::reference         reference;
	typedef base_container::size_type         size_type;
	typedef base_container::difference_type   difference_type;

	typedef base_container::const_iterator          const_iterator;
	typedef base_container::iterator                iterator;
	typedef base_container::const_reverse_iterator  const_reverse_iterator;
	typedef base_container::reverse_iterator        reverse_iterator;

private:
	base_container m_elements;

public:
	Optional_map()
		: m_elements()
	{
		//
	}
#ifndef POSEIDON_CXX11
	Optional_map(const Optional_map &rhs)
		: m_elements(rhs.m_elements)
	{
		//
	}
	Optional_map &operator=(const Optional_map &rhs){
		m_elements = rhs.m_elements;
		return *this;
	}
#endif

public:
	bool empty() const {
		return m_elements.empty();
	}
	size_type size() const {
		return m_elements.size();
	}
	void clear(){
		m_elements.clear();
	}

	const_iterator begin() const {
		return m_elements.begin();
	}
	iterator begin(){
		return m_elements.begin();
	}
	const_iterator cbegin() const {
		return m_elements.begin();
	}
	const_iterator end() const {
		return m_elements.end();
	}
	iterator end(){
		return m_elements.end();
	}
	const_iterator cend() const {
		return m_elements.end();
	}

	const_reverse_iterator rbegin() const {
		return m_elements.rbegin();
	}
	reverse_iterator rbegin(){
		return m_elements.rbegin();
	}
	const_reverse_iterator crbegin() const {
		return m_elements.rbegin();
	}
	const_reverse_iterator rend() const {
		return m_elements.rend();
	}
	reverse_iterator rend(){
		return m_elements.rend();
	}
	const_reverse_iterator crend() const {
		return m_elements.rend();
	}

	iterator erase(const_iterator pos){
		return m_elements.erase(pos);
	}
	iterator erase(const_iterator first, const_iterator last){
		return m_elements.erase(first, last);
	}
	size_type erase(const char *key){
		return erase(Shared_nts::view(key));
	}
	size_type erase(const Shared_nts &key){
		return m_elements.erase(key);
	}

	void swap(Optional_map &rhs) NOEXCEPT {
		using std::swap;
		swap(m_elements, rhs.m_elements);
	}

	// 一对一的接口。
	const_iterator find(const char *key) const {
		return find(Shared_nts::view(key));
	}
	const_iterator find(const Shared_nts &key) const {
		return m_elements.find(key);
	}
	iterator find(const char *key){
		return find(Shared_nts::view(key));
	}
	iterator find(const Shared_nts &key){
		return m_elements.find(key);
	}

	bool has(const char *key) const {
		return find(key) != end();
	}
	bool has(const Shared_nts &key){
		return find(key) != end();
	}
	iterator set(Shared_nts key, std::string val){
		AUTO(pair, m_elements.equal_range(key));
		if(pair.first == pair.second){
			return m_elements.emplace(STD_MOVE_IDN(key), STD_MOVE_IDN(val));
		} else {
			pair.first = m_elements.erase(pair.first, --pair.second);
			pair.first->second.swap(val);
			return pair.first;
		}
	}

	const std::string &get(const char *key) const { // 若指定的键不存在，则返回空字符串。
		return get(Shared_nts::view(key));
	};
	const std::string &get(const Shared_nts &key) const {
		const AUTO(it, find(key));
		if(it == end()){
			return empty_string();
		}
		return it->second;
	}
	const std::string &at(const char *key) const { // 若指定的键不存在，则抛出 std::out_of_range。
		return at(Shared_nts::view(key));
	};
	const std::string &at(const Shared_nts &key) const {
		const AUTO(it, find(key));
		if(it == end()){
			throw std::out_of_range(__PRETTY_FUNCTION__);
		}
		return it->second;
	}
	std::string &at(const char *key){ // 若指定的键不存在，则抛出 std::out_of_range。
		return at(Shared_nts::view(key));
	};
	std::string &at(const Shared_nts &key){
		const AUTO(it, find(key));
		if(it == end()){
			throw std::out_of_range(__PRETTY_FUNCTION__);
		}
		return it->second;
	}

	// 一对多的接口。
	std::pair<const_iterator, const_iterator> range(const char *key) const {
		return range(Shared_nts::view(key));
	}
	std::pair<const_iterator, const_iterator> range(const Shared_nts &key) const {
		return m_elements.equal_range(key);
	}
	std::pair<iterator, iterator> range(const char *key){
		return range(Shared_nts::view(key));
	}
	std::pair<iterator, iterator> range(const Shared_nts &key){
		return m_elements.equal_range(key);
	}
	size_type count(const char *key) const {
		return count(Shared_nts::view(key));
	}
	size_type count(const Shared_nts &key) const {
		return m_elements.count(key);
	}

	iterator append(Shared_nts key, std::string val){
		return m_elements.emplace(STD_MOVE_IDN(key), STD_MOVE_IDN(val));
	}
};

inline void swap(Optional_map &lhs, Optional_map &rhs) NOEXCEPT {
	lhs.swap(rhs);
}

extern std::ostream &operator<<(std::ostream &os, const Optional_map &rhs);

}

#endif
