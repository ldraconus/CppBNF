#include "BNF.h"

using namespace std;

namespace BNF {
    bool Node::Eval(State& state) {
	switch (type) {
	case ASSIGN: return rhs->Eval(state);
	case NOOP:   return lhs->Eval(state);
	case AND: 
	    {
		State savedState(state);
		if (lhs->Eval(state) && rhs->Eval(state)) {
		    string b = state.Pop();
		    string a = state.Pop();
		    if (b == "" && a == "") state.Save("");
		    else if (a == "") state.Save(b);
		    else if (b == "") state.Save(a);
		    else state.Save(a + " " + b);
		    return true;
		}
		state = savedState;
		return false;
	    }
	    break;
	case OR:
	    {
		State savedState(state);
		if (lhs->Eval(state)) return true;
		state = savedState;
		if (rhs->Eval(state)) return true;
		state = savedState;
		return false;
	    }
	    break;
	}

	return false;
    }

    bool Token::Eval(State& state) {
	smatch m;
	state.SkipWS();
	string& input = state.Input();
	if (regex_search(input, m, pat)) {
	    if (m.str() != "") state.Save(m.str());
	    input = m.suffix();
	    return true;
	}
	return false;
    }
}