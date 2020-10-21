#include "common/common.hpp"

#include "common/combinate.hpp"

#include <cctype>

namespace cpparse::common {

Parser<char> Any() {
	return Parser<char>([](std::stringstream& in) -> ParseResult<char> {
		char c;
		if (!(in >> c)) {
			return ParseResult<char>::empty(in, "Stream has no more characters to consume.");
		}
		return ParseResult<char>::with(c, in);
	});
};

Parser<char> Char(char c) {
	return (Any() >>= Satisfies<char>([c](char d) { return c == d; })) |
		   Failure<char>("Expected character '" + std::string(1, c) + "'");
}

Parser<char> OneOf(const std::string&& opts) {
	if (opts.empty()) return Failure<char>("No options given to OneOf");
	return Char(opts[0]) | OneOf(opts.substr(1)) |
		   Failure<char>("Could not match character with any of '" + opts + "'");
}

Parser<char> Alpha() {
	return (Any() >>= Satisfies<char>(isalpha)) |
		   Failure<char>("Expected an alphabetical character.");
}

Parser<char> Numeric() {
	return (Any() >>= Satisfies<char>(isdigit)) |
		   Failure<char>("Expected digit.");
}

Parser<unsigned int> Digit() {
	return Transform<char, unsigned int>(
		Numeric(),
		[](char c) { return c - '0'; });
}

Parser<char> AlphaNumeric() {
	return Alpha() | Numeric() | Failure<char>("Expected alphanumeric.");
}

Parser<std::string> UIntegerS() {
	return Many1<char, std::string>(Numeric()) |
		   Failure<std::string>("Expected unsigned integer.");
}

Parser<unsigned int> UInteger() {
	return To<unsigned int>(UIntegerS());
}

Parser<std::string> IntegerS() {
	return (Char('-') > UIntegerS()) | UIntegerS() |
		   Failure<std::string>("Expected integer.");
}

Parser<int> Integer() {
	return To<int>(IntegerS());
}

Parser<std::string> NumberS() {
	return ((IntegerS() | UIntegerS() | String("-") | Const<std::string>("0")) <=
			((Char('.') > UIntegerS()) | Const<std::string>(""))) |
		   Failure<std::string>("Expected number.");
}

Parser<double> Number() {
	return To<double>(NumberS());
}

Parser<std::string> Whitespace() {
	return Many<char, std::string>(Any() >>= Satisfies<char>(isspace));
}

Parser<std::string> String(const std::string&& str) {
	if (str.size() == 0) {
		return Const<std::string>("");
	}
	return (Char(str[0]) > String(str.substr(1))) |
		   Failure<std::string>("Could not match string '" + str + "'");
}

Parser<std::string> OfLength(Parser<std::string> p, size_t len) {
	return (p >>=
			Satisfies<std::string>(
				[len](std::string c) { return c.size() == len; })) |
		   Failure<std::string>(
			   "Expected a match of length " + std::to_string(len) + ".");
}

}
