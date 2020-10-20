#pragma once

#include <functional>
#include <parseresult.hpp>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "util/parseerror.hpp"

namespace cpparse {

// parser internal function
template <typename T, typename... Args>
using ParserF = std::function<ParseResult<T>(std::stringstream&, Args...)>;

typedef std::stringstream::pos_type spos_t;

//base parser class
template <typename T, typename... Args>
class Parser {
public:
	//init
	Parser() {}
	Parser(ParserF<T, Args...> fn) : mComputation(fn) {}
	virtual ~Parser() {}

	// main parser operator.throws std::parseerror if an error occurs
	ParseResult<T> operator()(std::stringstream& in, Args... args) const {
		if (!mComputation) {
			throw util::parseerror("No implementation defined!");
		}
		spos_t ipos		 = in.tellg();
		ParseResult<T> r = mComputation(in, args...);
		if (!r) {
			in.seekg(ipos);
		}
		return r;
	}

private:
	// internal function for computation
	ParserF<T, Args...> mComputation;
};

// combine the results of two heterogenous parsers into one tuple
template <typename T, typename U>
Parser<std::tuple<T, U>> operator+(Parser<T> lhs, Parser<U> rhs) {
	return Parser<std::tuple<T, U>>([lhs, rhs](std::stringstream& in) {
		ParseResult<T> r1 = lhs(in);
		if (!r1) {
			return ParseResult<std::tuple<T, U>>::empty(in, r1.error());
		}
		ParseResult<U> r2 = rhs(r1.rest());
		if (!r2) {
			return ParseResult<std::tuple<T, U>>::empty(in, r2.error());
		}
		return ParseResult<std::tuple<T, U>>::with(std::make_tuple(r1.res(), r2.res()), r2.rest());
	});
}

// combine the results of two heterogenous parsers into one tuple, passing the first parser's output as an argument to the second parser.
template <typename T, typename U>
Parser<std::tuple<T, U>> operator+=(Parser<T> lhs, Parser<U, T> rhs) {
	return Parser<std::tuple<T, U>>([lhs, rhs](std::stringstream& in) {
		ParseResult<T> r1 = lhs(in);
		if (!r1) {
			return ParseResult<std::tuple<T, U>>::empty(in, r1.error());
		}
		ParseResult<U> r2 = rhs(r1.rest(), r1.res());
		if (!r2) {
			return ParseResult<std::tuple<T, U>>::empty(in, r2.error());
		}
		return ParseResult<std::tuple<T, U>>::with(std::make_tuple(r1.res(), r2.res()), r2.rest());
	});
}

// combine the results of two homogenous parsers into a container
template <typename T, typename Container = std::vector<T>>
Parser<Container> operator&(Parser<T> lhs, Parser<T> rhs) {
	return Parser<Container>([lhs, rhs](std::stringstream& in) {
		ParseResult<T> r1 = lhs(in);
		if (!r1) {
			return ParseResult<Container>::empty(in, r1.error());
		}
		ParseResult<T> r2 = rhs(r1.rest());
		if (!r2) {
			return ParseResult<Container>::empty(in, r2.error());
		}
		Container c;
		c.insert(c.end(), r1.res());
		c.insert(c.end(), r2.res());
		return ParseResult<Container>::with(c, r2.rest);
	});
}

// combine the results of two homogenous parsers into a vector, passing the result of the first into the args of the second
template <typename T, typename Container = std::vector<T>>
Parser<Container> operator&=(Parser<T> lhs, Parser<T, T> rhs) {
	return Parser<Container>([lhs, rhs](std::stringstream& in) {
		ParseResult<T> r1 = lhs(in);
		if (!r1) {
			return ParseResult<Container>::empty(in, r1.error());
		}
		ParseResult<T> r2 = rhs(r1.rest(), r1.res());
		if (!r2) {
			return ParseResult<Container>::empty(in, r2.error());
		}
		Container c;
		c.insert(c.end(), r1.res());
		c.insert(c.end(), r2.res());
		return ParseResult<Container>::with(c, r2.rest);
	});
}

// append a parser to the results of a list parser.
template <typename T, typename Container = std::vector<T>>
Parser<Container> operator<(Parser<Container> lhs, Parser<T> rhs) {
	return Parser<Container>([lhs, rhs](std::stringstream& in) {
		ParseResult<Container> r1 = lhs(in);
		if (!r1) {
			return ParseResult<Container>::empty(in, r1.error());
		}
		ParseResult<T> r2 = rhs(r1.rest());
		if (!r2) {
			return ParseResult<Container>::empty(in, r2.error());
		}
		Container rtot = r1.res();
		rtot.insert(rtot.end(), r2.res());
		return ParseResult<Container>::with(rtot, r2.rest);
	});
}

// prepend a parser to the results of a list parser.
template <typename T, typename Container = std::vector<T>>
Parser<Container> operator>(Parser<T> lhs, Parser<Container> rhs) {
	return Parser<Container>([lhs, rhs](std::stringstream& in) {
		ParseResult<T> r1 = lhs(in);
		if (!r1) {
			return ParseResult<Container>::empty(in, r1.error());
		}
		ParseResult<Container> r2 = rhs(r1.rest());
		if (!r2) {
			return ParseResult<Container>::empty(in, r2.error());
		}
		Container rtot = r2.res();
		rtot.insert(rtot.begin(), r1.res());
		return ParseResult<Container>::with(rtot, r2.rest());
	});
}

// append two list parsers together.
template <typename Container>
Parser<Container> operator<=(Parser<Container> lhs, Parser<Container> rhs) {
	return Parser<Container>([lhs, rhs](std::stringstream& in) {
		ParseResult<Container> r1 = lhs(in);
		if (!r1) {
			return ParseResult<Container>::empty(in, r1.error());
		}
		ParseResult<Container> r2 = rhs(r1.rest());
		if (!r2) {
			return ParseResult<Container>::empty(in, r2.error());
		}
		Container rtot = r1.res();
		rtot.insert(rtot.end(), r2.res().begin(), r2.res().end());
		return ParseResult<Container>::with(rtot, r2.rest);
	});
}

// prepend two list parsers together
template <typename Container>
Parser<Container> operator>=(Parser<Container> lhs, Parser<Container> rhs) {
	return Parser<Container>([lhs, rhs](std::stringstream& in) {
		ParseResult<Container> r1 = lhs(in);
		if (!r1) {
			return ParseResult<Container>::empty(in, r1.error());
		}
		ParseResult<Container> r2 = rhs(r1.rest());
		if (!r2) {
			return ParseResult<Container>::empty(in, r2.error());
		}
		Container rtot = r2.res();
		rtot.insert(rtot.begin(), r1.res().begin(), r1.res().end());
		return ParseResult<Container>::with(rtot, r2.rest());
	});
}

// get the results from both parsers and only return the last
template <typename T, typename U>
Parser<U> operator>>(Parser<T> lhs, Parser<U> rhs) {
	return Parser<U>([lhs, rhs](std::stringstream& in) {
		ParseResult<T> r1 = lhs(in);
		if (!r1) {
			return ParseResult<U>::empty(in, r1.error());
		}
		ParseResult<U> r2 = rhs(r1.rest());
		if (!r2) {
			return ParseResult<U>::empty(in, r2.error());
		}
		return r2;
	});
}

// get the results from both parsers and only return the last, passing the result of the previous parser into the next parser
template <typename T, typename U>
Parser<U> operator>>=(Parser<T> lhs, Parser<U, T> rhs) {
	return Parser<U>([lhs, rhs](std::stringstream& in) {
		ParseResult<T> r1 = lhs(in);
		if (!r1) {
			return ParseResult<U>::empty(in, r1.error());
		}
		ParseResult<U> r2 = rhs(r1.rest(), r1.res());
		if (!r2) {
			return ParseResult<U>::empty(in, r2.error());
		}
		return r2;
	});
}

// provide an alternative parser in case the first fails
template <typename T>
Parser<T> operator|(Parser<T> fst, Parser<T> fallback) {
	return Parser<T>([fst, fallback](std::stringstream& in) {
		spos_t ipos		  = in.tellg();
		ParseResult<T> r1 = fst(in);
		if (!r1) {
			in.seekg(ipos);
			ParseResult<T> r2 = fallback(in);
			if (!r2) {
				return ParseResult<T>::empty(in, r2.error());
			}
			return r2;
		}
		return r1;
	});
}

}
