#pragma once
#include<vector>
#include<string>
#include<set>
#include<map>
#include<filesystem>
#include<iostream>
#include<fstream>

#include"Vlpp.h"
#include"RegExpParser.h"

namespace hscp {
	struct state;
	struct transition;
	// a state in automaton
	struct state {
		//std::vector<transition*> ins; // transition comes in
		std::vector<transition*> trans; // transition functions
		bool finalState; // is final
		std::string is; // the lexical meaning which this function belongs to
	};
	// a transition in automaton; both used to record a fragment in automaton
	struct transition {
		CharRange input; // function input
		std::string is; // the lexical meaning which this function belongs to
		state* from, * to; // where the function comes and goes
	};
	// a automaton
	class Automaton {
	public:
		std::vector<vl::Ptr<state>> states;
		std::vector<vl::Ptr<transition>> transitions;
		std::set<CharRange> inputs;
		state* startState = nullptr; // the very beginning of this automaton

		// add a state
		state* NewState(const std::string& is = "") {
			state* t = new state{ {},false, is };
			states.push_back(t);
			return t;
		}
		// add a transition
		transition* NewTransition(state* from, state* to, CharRange chr, const std::string& is) {
			transition* t = new transition{ chr,is,from,to };
			from->trans.push_back(t);
			//to->ins.push_back(t);
			transitions.push_back(t);
			return t;
		}
		/*transition* NewTransition(state* from, transition* t) {
			from->trans.push_back(t);
			transitions.push_back(t);
			return t;
		}*/
		// add a transition takes epsilon
		transition* NewEpsilon(state* from, state* to) {
			return NewTransition(from, to, 0, "");
		}
		// print all transitions
		void print() {
			for (const auto& t : transitions) {
				std::cout << t->from << "  [" << t->input << "]  " << t->to << "  " << (t->to->finalState) << '\n';
			}
		}
		// dump this automaton to file
//		void Dump() {
//#ifdef _DEBUG
//			constexpr auto route = "C:\\Users\\Miracle\\Source\\Repos\\CompilePrinciple\\Debug\\auto.bak.bin";
//#else
//			constexpr auto route = "auto.bak.bin";
//#endif
//			if (std::filesystem::exists(route)) { // if exists
//				std::cout << "\n***************************\n"
//					<< "Dump file already exists, continue? (Y/n) ";
//				char k; std::cin.get(k);
//				if (k != 'Y' && k != 'y') { // whether cover the file or not
//					std::cout << "\nOperation cancelled.\n";
//					return;
//				}
//			}
//			std::map<state*, int> record;
//			for (const auto& s : states) {
//				record.emplace(s.Obj(), record.size() + 1); // generate a state id
//			}
//			int divider = 0, tt = 0;
//			std::ofstream fout(route, std::ios::binary | std::ios::trunc);
//			for (const auto& r : record) { // states
//				fout.write(reinterpret_cast<const char*>(&(r.second)), sizeof(int)); // id
//				tt = r.first->finalState;
//				fout.write(reinterpret_cast<const char*>(&tt), sizeof(char)); // is final
//			}
//			fout.write(reinterpret_cast<const char*>(&divider), sizeof(int)); // divider
//			// start state
//			fout.write(reinterpret_cast<const char*>(&record[startState]), sizeof(int)); // id
//			//fout.write(reinterpret_cast<const char*>((int)0), sizeof(int)); // divider
//
//			for (const auto& t : transitions) { // transitions
//				fout.write(reinterpret_cast<const char*>(&record[t->from]), sizeof(int));
//				fout.write(reinterpret_cast<const char*>(&record[t->to]), sizeof(int));
//				tt = t->input;
//				fout.write(reinterpret_cast<const char*>(&tt), sizeof(char));
//
//				const char* s = t->is == "" ? "*" : t->is.c_str(); // * stands for nothing
//				tt = strlen(s);
//				fout.write(reinterpret_cast<const char*>(&tt), sizeof(int)); // symbol counts
//				fout.write(s, strlen(s));
//			}
//			fout.write(reinterpret_cast<const char*>(&divider), sizeof(char)); // end
//
//			fout.flush(); fout.close();
//			std::cout << "\nOperation success.\n";
//		}
		// generate nfa from postfix regular expression
		static Automaton RegexPost2NFA(const std::vector<regextok>& regex, const std::string& is) {
			std::stack<transition*> subAutos; // a stack for fragments
			auto newSubs = [](state* begin, state* end) { // local function to create a fragment
				transition* t = new transition{ 255,"",begin,end };
				return t;
			};
			Automaton nfa;

			transition* sub1, * sub2;
			state* st, * ed;
			for (auto i = regex.begin(); i != regex.end(); ++i) {
				if (*i == '|') { // parallel two fragments
					sub1 = subAutos.top(); subAutos.pop();
					sub2 = subAutos.top(); subAutos.pop();

					st = nfa.NewState();
					ed = nfa.NewState();

					nfa.NewEpsilon(st, sub1->from);
					nfa.NewEpsilon(st, sub2->from);
					nfa.NewEpsilon(sub1->to, ed);
					nfa.NewEpsilon(sub2->to, ed);

					subAutos.push(newSubs(st, ed));
				}
				else if (*i == '&') { // concatenate two fragments
					sub1 = subAutos.top(); subAutos.pop(); // right
					sub2 = subAutos.top(); subAutos.pop(); // left

					nfa.NewEpsilon(sub2->to, sub1->from);

					subAutos.push(newSubs(sub2->from, sub1->to));
				}
				else if (*i == '*') { // closure
					sub1 = subAutos.top(); subAutos.pop();

					st = nfa.NewState();
					ed = nfa.NewState();

					nfa.NewEpsilon(st, ed);
					nfa.NewEpsilon(st, sub1->from);
					nfa.NewEpsilon(sub1->to, ed);
					nfa.NewEpsilon(sub1->to, sub1->from);

					subAutos.push(newSubs(st, ed));
				}
				else if (*i == '+') { // positive closure
					sub1 = subAutos.top(); subAutos.pop();

					st = nfa.NewState();
					ed = nfa.NewState();

					nfa.NewEpsilon(st, sub1->from);
					nfa.NewEpsilon(sub1->to, ed);
					nfa.NewEpsilon(sub1->to, sub1->from);

					subAutos.push(newSubs(st, ed));
				}
				else if (*i == '?') { // optional
					sub1 = subAutos.top(); subAutos.pop();

					st = nfa.NewState();
					ed = nfa.NewState();

					nfa.NewEpsilon(st, ed);
					nfa.NewEpsilon(st, sub1->from);
					nfa.NewEpsilon(sub1->to, ed);

					subAutos.push(newSubs(st, ed));
				}
				else // normal character
				{
					st = nfa.NewState();
					ed = nfa.NewState();

					nfa.NewTransition(st, ed, *i, is);

					subAutos.push(newSubs(st, ed));
				}
			}

			nfa.startState = subAutos.top()->from; // set start
			subAutos.top()->to->finalState = true; // set finish
			subAutos.top()->to->is = is;
			return std::move(nfa);
		}
		// merge two parallel automaton
		static Automaton Merge(Automaton& nfa1, Automaton& nfa2) {
			Automaton nfa;
			auto st = nfa.NewState();
			nfa.startState = st;

			if (nfa1.startState != nullptr) { // check empty automaton
				nfa.NewEpsilon(st, nfa1.startState);
				std::copy(nfa1.states.begin(), nfa1.states.end(), std::back_inserter(nfa.states));
				std::copy(nfa1.transitions.begin(), nfa1.transitions.end(), std::back_inserter(nfa.transitions));
			}
			if (nfa2.startState != nullptr) { // check empty automaton
				nfa.NewEpsilon(st, nfa2.startState);
				std::copy(nfa2.states.begin(), nfa2.states.end(), std::back_inserter(nfa.states));
				std::copy(nfa2.transitions.begin(), nfa2.transitions.end(), std::back_inserter(nfa.transitions));
			}

			return std::move(nfa);
		}
		// build automaton from dumped file
//		static Automaton ReadFromFile() {
//#ifdef _DEBUG
//			constexpr auto route = "C:\\Users\\Miracle\\Source\\Repos\\CompilePrinciple\\Debug\\auto.bak.bin";
//#else
//			constexpr auto route = "auto.bak.bin";
//#endif
//			if (!std::filesystem::exists(route)) {
//				std::cout << "\nDump file not found.\n";
//				return Automaton();
//			}
//			Automaton at;
//			std::ifstream fin(route, std::ios::binary);
//
//			int num = 0;
//			std::map<int, state*> record;
//			char str[2000];
//			while (true) // states
//			{
//				fin.read(reinterpret_cast<char*>(&num), sizeof(int)); // id or divider
//				if (num == 0) break; // end states
//				auto last = record.emplace(num, at.NewState());
//				fin.read(reinterpret_cast<char*>(&num), sizeof(char)); // read final
//
//				last.first->second->finalState = num;
//			}
//			fin.read(reinterpret_cast<char*>(&num), sizeof(int)); // start state id
//			at.startState = record[num];
//			while (true)
//			{
//				fin.read(reinterpret_cast<char*>(&num), sizeof(int)); // id or divider
//				if (num == 0) break; // end transitions
//				state* from = record[num], * to;
//				fin.read(reinterpret_cast<char*>(&num), sizeof(int)); // id
//				to = record[num];
//				fin.read(reinterpret_cast<char*>(&num), sizeof(char)); // input
//				auto last = at.NewTransition(from, to, (unsigned char)num, "");
//
//				fin.read(reinterpret_cast<char*>(&num), sizeof(int)); // str count
//				fin.read(str, num);
//				str[num] = 0;
//
//				if (strcmp(str, "*") != 0) // set property
//				{
//					last->is = str;
//				}
//			}
//
//			return std::move(at);
//		}
	};
}