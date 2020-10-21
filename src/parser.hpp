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
template <typename T>
using ParserF = std::function<ParseResult<T>(std::stringstream&)>;

typedef std::stringstream::pos_type spos_t;

// For saving and retrieving parser values hidden in a parser chain.
template <typename T>
class Parser;
template <typename T>
class ParseResultFuture {
public:
	ParseResultFuture() : mBound(nullptr) {}
	~ParseResultFuture() {
		if (bound()) {
			delete mBound;
		}
	}

	operator bool() const {
		return bound();
	}

	T& result() {
		if (!bound()) throw std::runtime_error("Attempt to read from not yet received parser result");
		return mBound->res();
	}

private:
	friend class Parser<T>;

	void bind(ParseResult<T>& p) {
		if (bound()) delete mBound;
		mBound = new ParseResult<T>(p.mRes, p.mRest, p.mErr);
	}

	bool bound() const {
		return mBound != nullptr;
	}

	// the bound parser result
	ParseResult<T>* mBound;
};

// base parser class
template <typename T>
class Parser {
public:
	//init
	Parser() : mBinds() {}
	Parser(ParserF<T> fn) : mComputation(fn), mBinds() {}
	virtual ~Parser() {}

	// Bind a variable to be set once the parser returns.
	Parser<T>& bind(ParseResultFuture<T>* v) {
		mBinds.push_back(v);
		return *this;
	}

	// main parser operator. throws std::parseerror if an error occurs
	ParseResult<T> operator()(std::stringstream& in) const {
		if (!mComputation) {
			throw util::parseerror("No implementation defined!");
		}
		spos_t ipos		 = in.tellg();
		ParseResult<T> r = mComputation(in);
		if (!r) {
			in.seekg(ipos);
		}
		// update all bound futures
		for (auto& i : mBinds) {
			i->bind(r);
		}
		return r;
	}

	// is the parser valid?
	operator bool() const {
		return (bool)mComputation;
	}

private:
	// internal function for computation
	ParserF<T> mComputation;
	// all binds
	std::vector<ParseResultFuture<T>*> mBinds;
};

template <typename T, typename... Args>
using ParserG = std::function<Parser<T>(Args...)>;

// get the results from both parsers and only return the last
template <typename T, typename U>
Parser<U> operator>>(const Parser<T>& lhs, const Parser<U>& rhs) {
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
Parser<U> operator>>=(const Parser<T>& lhs, const ParserG<U, T>& rhs) {
	return Parser<U>([lhs, rhs](std::stringstream& in) {
		ParseResult<T> r1 = lhs(in);
		if (!r1) {
			return ParseResult<U>::empty(in, r1.error());
		}
		Parser<U> r2f = rhs(r1.res());
		if (!r2f) {
			return ParseResult<U>::empty(in, "operator >>= ParseG returned nothing");
		}
		ParseResult<U> r2 = r2f(r1.rest());
		return r2;
	});
};

// combine the results of two homogenous parsers into a container
template <typename T, typename Container = std::vector<T>>
Parser<Container> operator&(const Parser<T>& lhs, const Parser<T>& rhs) {
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

// append a parser to the results of a list parser.
template <typename T, typename Container = std::vector<T>>
Parser<Container> operator<(const Parser<Container>& lhs, const Parser<T>& rhs) {
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
Parser<Container> operator>(const Parser<T>& lhs, const Parser<Container>& rhs) {
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
Parser<Container> operator<=(const Parser<Container>& lhs, const Parser<Container>& rhs) {
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
		return ParseResult<Container>::with(rtot, r2.rest());
	});
}

// provide an alternative parser in case the first fails
template <typename T>
Parser<T> operator|(const Parser<T>& fst, const Parser<T>& fallback) {
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
