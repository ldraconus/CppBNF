// BNFtest.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stack>

#include <BNF.h>

using namespace std;
using namespace BNF;

#define STR(x) #x

struct LIST {
    void* car;
    void* cdr;
    bool isAtom;

    LIST() :                             car(NULL),               cdr(NULL), isAtom(false) { }
    LIST(const char* atm) :              car(_strdup(atm)),       cdr(NULL), isAtom(true)  { }
    LIST(const string s) :               car(_strdup(s.c_str())), cdr(NULL), isAtom(true)  { }
    LIST(void* const a, void* const b) : car(a),                  cdr(b),    isAtom(false) { }

    void* Car() { return car; }
    void* Cdr() { return cdr; }

    string str() {
	LIST* cr = (LIST*)car;
	LIST* cd = (LIST*)cdr;
	if (cr == NULL) return string("??");
	if (isAtom) return string((char*)cr);
	if (cd && cr->isAtom && cd->isAtom) return cr->str() + "." + cd->str();
	if (cd == NULL) {
	    if (cr->isAtom) return cr->str();
	    else if (cr->car && cr->cdr && ((LIST*)(cr->car))->isAtom && ((LIST*)(cr->cdr))->isAtom) return cr->str();
	    else return "(" + cr->str() + ")";
	}
	if (cr->isAtom) return cr->str() + " " + cd->str();
	if (cr->car && cr->cdr && ((LIST*)(cr->car))->isAtom && ((LIST*)(cr->cdr))->isAtom) return cr->str() + " " + cd->str();
	return string("(") + cr->str() + ")" + " " + cd->str();
    }
};

void Print(LIST* lst) {
    if (lst == NULL) printf("nil\n");
    else {
	LIST* car = (LIST*)lst->car;
	LIST* cdr = (LIST*)lst->cdr;
	if (lst->isAtom) printf("%s\n", lst->str().c_str());
	else if (car && cdr && car->isAtom && cdr->isAtom) printf("%s\n", lst->str().c_str());
	else printf("(%s)\n", lst->str().c_str());
    }
}

void Print(stack<LIST*>& stk) {
    while (!stk.empty()) {
	Print(stk.top());
	stk.pop();
    }
}

LIST* CONS(LIST* lst, LIST* tail)
{
    return new LIST(lst, tail);
}

static stack<LIST*> Stack;

class PAREN : public Action {
public:
    virtual bool Eval(State& state) { Stack.push(NULL); return true; }
};

class DOT : public Action {
public:
    virtual bool Eval(State& state) {
	LIST* b = Stack.top();
	Stack.pop();
	LIST* a = Stack.top();
	Stack.pop();
	Stack.push(new LIST(a, b));
	return true;
    }
};

class THESIS : public Action {
public:
    virtual bool Eval(State& state) {
	LIST* lst = (LIST*)NULL;
	LIST* p = NULL;
	while ((p = Stack.top()) != NULL) {
	    lst = new LIST(p, lst);
	    Stack.pop();
	}
	Stack.pop();
	Stack.push(lst);
	return true;
    }
};

class WORD : public Action {
    virtual bool Eval(State& state) { Stack.push(new LIST(state.Matched(1))); return true; }
};

string partialMsg(string rem) {
    return string("PARTIAL (left: '") + rem + "')";
}

string msg(bool res, bool partial, string tst, string remaining) {
    if (res) return partial ? partialMsg(remaining).c_str() : "MATCH";
    return (remaining == tst) ? "NO MATCH" : partialMsg(remaining + "'");
}

bool testCase(Rule& r, string tst, bool expect = true, bool partial_ok = false) {
    State state(tst);
    bool partial = false;
    bool res = r.Eval(state);
    string remaining = state.Input();
    if (!partial_ok) res &= remaining.length() == 0;
    else partial = remaining.length() != 0;
    printf("[%s] '%s' = %s ", (expect == res) ? "PASS" : "FAIL", tst.c_str(), msg(res, partial, tst, remaining).c_str());
    if (res) Print(Stack);
    else printf("\n");
    while (!Stack.empty()) Stack.pop();
    return res;
}

int main()
{
    Rule start;
    Rule atom;
    Rule item;
    Rule items;
    Rule list;
    Rule atomic;
    Rule opt_tail;
    WORD Word;
    DOT Dot;
    PAREN Paren;
    THESIS Thesis;
    Token ATOM("[0-9a-zA-Z]+");

    start <<= item;

    items <<= item + items
	   |  ""
           ;

    item <<= atomic
	| list
	;

    atomic <<= atom + opt_tail;

    opt_tail <<= "[.]" + atom + Dot
	      |  ""
	      ;

    atom <<= ATOM + Word;

    list <<= "[(]" + Paren + items + "[)]" + Thesis;

    bool success = true;

    success &= testCase(start, "a");
    success &= testCase(start, "a.b");
    success &= testCase(start, "()");
    success &= !testCase(start, ")(", false);
    success &= !testCase(start, "(a & b)", false);
    success &= testCase(start, "(a)");
    success &= testCase(start, "(a.b)");
    success &= testCase(start, "(a b)");
    success &= testCase(start, "(a b.c)");
    success &= testCase(start, "(a (b) c)");
    success &= testCase(start, "(a (b.d) c)");
    success &= testCase(start, "(a (b) c) (a b)", true, true);
    success &= testCase(start, "(a (b) c.d) (a b)", true, true);
    printf("=================================================================\n");
    if (success) printf("[PASS] All tests passed\n");
    else printf("[FAIL] Not all tests passed\n");

    (void) getchar();

    return 0;
}

