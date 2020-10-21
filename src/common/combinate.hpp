#pragma once

#include <common/common.hpp>
#include <common/transform.hpp>
#include <parser.hpp>
#include <parseresult.hpp>

#include <functional>
#include <vector>

// Parser combinators.
namespace cpparse::common {

// Match 1 or more of a parser
template <typename T, typename Container = std::vector<T>>
Parser<Container> Many1(Parser<T> p) {
	return Parser<Container>([p](std::stringstream& in) {
		std::function<ParseResult<Container>(std::stringstream&)> fn = [p, &fn](std::stringstream& in) {
			ParseResult<T> fst = p(in);
			if (!fst) {
				return ParseResult<Container>::empty(in, "Could not match anything.");
			}
			Container full;
			full.insert(full.end(), fst.res());

			ParseResult<Container> rest = fn(fst.rest());
			if (rest) {
				full.insert(full.end(), rest.res().begin(), rest.res().end());
			}

			return ParseResult<Container>::with(full, rest.rest());
		};
		return fn(in);
	});
}

// Match 0 or more of a parser
template <typename T, typename Container = std::vector<T>>
Parser<Container> Many(Parser<T> p) {
	return Many1<T, Container>(p) | Const(Container{});
}

// Match 0 or 1 of a parser.
template <typename T>
Parser<std::optional<T>> Maybe(Parser<T> p) {
	return Transform<T, std::optional<T>>(p, [](T t) { return std::optional(t); }) | Const(std::optional<T>());
}

// Sum up parser results using the + operator
template <typename T>
Parser<T> Sum(Parser<T> a, Parser<T> b) {
	return Parser<T>([a, b](std::stringstream& in) {
		ParseResult<T> r1 = a(in);
		if (!r1) {
			return r1;
		}
		ParseResult<T> r2 = b(r1.rest());
		if (!r2) {
			return r2;
		}
		return ParseResult<T>::with(r1.res() + r2.res(), r2.rest());
	});
}

}
