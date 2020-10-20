#include "parseerror.hpp"

namespace cpparse::util {

parseerror::parseerror(const std::string& what)
	: std::logic_error("PARSER ERROR: " + what) {
}

const char* parseerror::what() const noexcept {
	return std::logic_error::what();
}

}
