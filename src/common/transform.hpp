#pragma once

#include <common/common.hpp>
#include <parser.hpp>
#include <parseresult.hpp>
#include <util/parseerror.hpp>

#include <functional>
#include <sstream>
#include <vector>

// Parser Transforms
namespace cpparse::common {

// Transform the result of a parser
template <typename F, typename T, typename Fn>
Parser<T> Transform(Parser<F> p, Fn fn) {
	return Parser<T>([p, fn](std::stringstream& in) {
		ParseResult<F> r = p(in);
		if (!r) {
			return ParseResult<T>::empty(in, r.error());
		}
		try {
			return ParseResult<T>::with(fn(r.res()), r.rest());
		} catch (const std::exception& e) {
			return ParseResult<T>::empty(in, "Transform Error: " + std::string(e.what()));
		}
	});
}

// Transform the result of a parser.
template <typename F, typename T>
Parser<T> TransformL(Parser<F> p, std::function<T(F)> tf) {
	return Transform(p, tf);
}

// Directly cast the result of a parser.
template <typename F, typename T>
Parser<T> Cast(Parser<F> p) {
	return Transform<F, T>(p, [](F v) { return (T)v; });
}

// Cast a parser from string.
template <typename T>
Parser<T> To(Parser<std::string> p) {
	return Transform<std::string, T>(p, [](std::string v) -> T {
		std::stringstream ss;
		ss << v;
		T n;
		ss >> n;
		if (ss.fail()) {
			throw util::parseerror("Could not convert from '" + v + "'");
		}
		return n;
	});
}

// Cast a parser to a string
template <typename F>
Parser<std::string> From(Parser<F> p) {
	return Transform<F, std::string>(p, std::to_string);
}

// Remove an optional qualifier
template <typename T>
Parser<T> Guarantee(Parser<std::optional<T>>, T fb) {
	return Transform<std::optional<T>, T>([fb](std::optional<T> t) -> T {
		if (t) {
			return t.value();
		} else {
			return fb;
		}
	});
}

}
