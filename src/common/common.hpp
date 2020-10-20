#pragma once

#include "parser.hpp"

// Common universal parsers
namespace cpparse::common {

// Always fails.
template <typename T>
Parser<T> Failure(std::string err = "") {
	return Parser<T>([err](std::stringstream& in) {
		return ParseResult<T>::empty(in, err);
	});
}

// Always returns the given value, without mutating the stream.
template <typename T>
Parser<T> Const(T val) {
	return Parser<T>([val](std::stringstream& in) {
		return ParseResult<T>::with(val, in);
	});
}

// Tests a given parameter against a condition
template <typename T, typename Fn>
Parser<T, T> Satisfies(Fn pred) {
	return Parser<T, T>([pred](std::stringstream& in, T c) {
		if (pred(c)) {
			return ParseResult<T>::with(c, in);
		} else {
			return ParseResult<T>::empty(in, "Satisfies() Condition was not met.");
		}
	});
}

// Tests a given parameter against a condition
template <typename T>
Parser<T, T> SatisfiesL(std::function<bool(T)> pred) {
	return Parser<T, T>([pred](std::stringstream& in, T c) {
		if (pred(c)) {
			return ParseResult<T>::with(c, in);
		} else {
			return ParseResult<T>::empty(in, "Satisfies() Condition was not met.");
		}
	});
}

// Match any single character
Parser<char> Any();

// Matches a specific character
Parser<char> Char(char c);

// Matches one of the given characters
Parser<char> OneOf(const std::string&& opts);

// Matches any alphabetical character
Parser<char> Alpha();

// Matches any numerical character
Parser<char> Numeric();

// Matches any digit
Parser<unsigned int> Digit();

// Matches any alphanumeric character
Parser<char> AlphaNumeric();

// Matches a positive integer.
Parser<std::string> UIntegerS();
// Matches a positive integer.
Parser<unsigned int> UInteger();

Parser<std::string> IntegerS();
// Matches an integer.
Parser<int> Integer();

// Matches any number
Parser<std::string> NumberS();
// Matches any number.
Parser<double> Number();

// Matches an exact string.
Parser<std::string> String(const std::string&& str);

// Checks the length of a string parser
Parser<std::string> OfLength(Parser<std::string> p, size_t len);

}
