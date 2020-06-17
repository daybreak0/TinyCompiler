#include"LexFileLoader.h"
#include"Automaton.h"
#include"GrammarFileReader.h"
#include"LRAutos.h"
#include"LRAnalyzer.h"
#include "DFA.h"
#include "LexMatcher.h"
#include "SematicLoader.h"
#include "SematicProcesser.h"
using namespace std;
// get an automaton
hscp::Automaton getAutos() {
#ifdef _DEBUG
	constexpr auto route = "Data\\lex-define.txt";
#else
	constexpr auto route = "lex-define.txt";
#endif

	hscp::Automaton at;
	hscp::FileLoader(route, [](const auto& err) {}, [&at](const vector<hscp::token_define>& defs) {
		for (const auto& d : defs) {
			auto nfa = hscp::Automaton::RegexPost2NFA(hscp::RegexProcesser::ProcessRegex(d.expr), d.id);

			auto dfa = hscp::DFAConverter::Nfa2Dfa(nfa);

			auto mindfa = hscp::DFAminimizer(dfa);

			at = hscp::Automaton::Merge(at, mindfa);
		}
		});

	at = hscp::DFAConverter::Nfa2Dfa(at);  // there're epsilons and transitions accept same inputs after merge

	return std::move(at);
}
int main(int argc, char** argv) {
	string file = "Data\\source.txt";
	if (argc == 2)
		file = argv[1]; // source file from parameter
	else return 0;
	auto at = getAutos();
	hscp::Matcher mc(at);
	auto tokens = mc.ReadFile(file);

	hscp::GrammarLoader ld;
	ld.Print();
	ld.EnableLR(); // in GrammarFileReader.h , there's a constant identifies the start symbol for grammar
	ld.Print();
	auto lrat = hscp::LR1Automaton::Build(ld);
	//lrat.MergeLALR1();
	auto t = lrat.LR1Table();
	//vector<hscp::Token> tokens = { {"","id","a",0,1,1},{"","=","=",0,1,2},{"","*","*",0,1,3},{"","c","*",0,1,4},{"","d","6",0,1,5},{"","e","",0,1,6},{"","#","#",0,1,6} };
	//vector<hscp::Token> tokens = { {"","id","a",0,1,1},{"","+","=",0,1,2},{"","id","*",0,1,3},{"","*","id",0,1,4},{"","(","id",0,1,5},{"","id","id",0,1,6},{"",")","id",0,1,7},{"","#","#",0,1,8} };
	hscp::Analyzer ana(lrat, t, tokens);
	ana.PrintErrors();

	auto& atree = ana.GetAnalyzeTree();

	hscp::SematicLoader sematic;
	auto ast = hscp::SematicProcesser::AnalyzeToAST(sematic, atree);

	hscp::PrintAST(ast);
	return 0;
}