#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

#include "util/parseerror.hpp"

namespace cpparse {

template <typename T>
class ParseResultFuture;

// result from a parser
template <typename T>
class ParseResult {
public:
	// is the result of the parser valid?
	bool valid() const {
		return mRes.has_value();
	}

	bool hasError() const {
		return mErr.size() != 0;
	}

	std::string error() const {
		return hasError() ? mErr : "Error";
	}

	T& res() {
		if (!valid()) {
			throw util::parseerror("Attempt to retrieve the value of an invalid parser result.");
		}
		return mRes.value();
	}

	const std::optional<T>& resOpt() const {
		return mRes;
	}

	std::stringstream& rest() {
		return mRest;
	}

	std::stringstream& rest() const {
		return mRest;
	}

	static ParseResult<T> empty(std::stringstream& rest, std::string err = "") {
		return ParseResult(
			std::optional<T>(),
			rest,
			err);
	}

	static ParseResult<T> with(T res, std::stringstream& rest) {
		return ParseResult(
			std::optional<T>(res),
			rest,
			"");
	}

	operator bool() const {
		return valid();
	}

private:
	friend class ParseResultFuture<T>;

	ParseResult(std::optional<T> res, std::stringstream& rest, std::string err = "")
		: mRes(res), mRest(rest), mErr(err) {}


	// the result of the parse, empty if there was no match
	std::optional<T> mRes;
	// the stream post-parse
	std::stringstream& mRest;
	// optional error message
	std::string mErr;
};

#ifdef OVERLOAD_OSTREAMS

// for printing std::vector
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
	if (v.size() == 0) {
		os << "[]";
		return os;
	}
	os << "[" << v[0];
	for (size_t i = 1; i < v.size(); ++i) {
		os << ", " << v[i];
	}
	os << "]";
	return os;
}

// ostream printing for std::optional
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& v) {
	if (!v) {
		os << "{}";
	} else {
		os << "{" << v.value() << "}";
	}
	return os;
}

// ostream printing for parseresult
template <typename T>
std::ostream& operator<<(std::ostream& os, const ParseResult<T>& r) {
	if (!r) {
		os << "(" << r.error() << ")";
	} else {
		auto& rest = r.rest();
		os << "("
		   << r.resOpt() << ", "
		   << (rest.rdbuf()->in_avail() == 0 ? "<empty>" : rest.str().substr(r.rest().tellg()))
		   << ")";
	}
	return os;
};

#endif

}
