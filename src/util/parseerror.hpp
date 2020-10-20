#pragma once

#include <stdexcept>
#include <string>

namespace cpparse::util {

// generic parser exception type
class parseerror : public std::logic_error {
public:
	parseerror(const std::string& what);
	const char* what() const noexcept;

private:
	std::string m_what;
};

}
