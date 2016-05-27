#pragma once

#include <regex>
#include <string>
#include <vector>
#include <map>

namespace BNF {
    class StringStack {
    private:
	std::vector<std::string> stack;
	int top;

    public:
	StringStack() :                     top(0)     {                  }
	StringStack(const StringStack& s) : top(s.top) { stack = s.stack; }

	void push(std::string a)      { if (top >= (int)stack.size()) stack.push_back(a); else stack[top] = a; ++top;           }
	std::string pop()             { return top ? stack[--top] : "";                                                         }
	std::string operator[](int i) { return (i >= 0) ? ((i > top) ? "" : stack[i]) : ((top + i < 0) ? "" : stack[top + i]);  }
	void clear()                  { stack.clear(); top = 0;                                                                 }
    };

    class State {
    private:
	std::string input;
	std::regex ws;
	StringStack matched;

    public:
	State(const std::string s, const std::string w = "[:s:]") : input(s),       ws(std::string("^[" + w + "]"))                        { matched.clear(); }
	State(const State& s) :                                     input(s.input), ws(s.ws),                        matched(s.matched)    {                  }

	std::string& Input() { return input; }

	void SkipWS() { std::smatch m; if (std::regex_search(input, m, ws)) input = m.suffix(); }

	void Save(std::string s)   { matched.push(s);      }
	std::string Matched(int n) { return matched[-n];   }
	std::string Pop()          { return matched.pop(); }
    };

    typedef enum { NOOP, AND, OR, ASSIGN } Type;

    class Node {
    protected: 
	Node* lhs;
	Type type;
	Node* rhs;

    public:
	Node(Node* l, Type t, Node* r) : lhs(l),    type(t),    rhs(r)    { }
	Node(Node* l) :                  lhs(l),    type(NOOP), rhs(NULL) { }
	Node() :                         lhs(NULL), type(NOOP), rhs(NULL) { }

	virtual bool Eval(State& state);

	Node& operator<<=(Node& n) { rhs = &n; type = ASSIGN; return *this; }
    };

    typedef Node Rule;

    class Token : public Node {
    private:
	std::regex pat;

    public:
	Token(std::string s) : Node(), pat(std::string("^") + s) { lhs = this; }

	virtual bool Eval(State& state);

	operator Node&() { return *this; }
    };

    class Action : public Node {
    public:
	Action() : Node() { lhs = this; }

	virtual bool Eval(State& state) { return true; }

	operator Node&() { return *this; }
    };

    inline Node& operator+(Node& l, Node& r)       { return *(new Node(&l,           AND, &r));           }
    inline Node& operator+(std::string l, Node& r) { return *(new Node(new Token(l), AND, &r));           }
    inline Node& operator+(Node& l, std::string r) { return *(new Node(&l,           AND, new Token(r))); }
    inline Node& operator|(Node& l, Node& r)       { return *(new Node(&l,           OR,  &r));           }
    inline Node& operator|(std::string l, Node& r) { return *(new Node(new Token(l), OR,  &r));           }
    inline Node& operator|(Node& l, std::string r) { return *(new Node(&l,           OR,  new Token(r))); }
}