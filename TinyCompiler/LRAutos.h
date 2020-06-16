#pragma once
#include<vector>
#include<map>
#include<set>
#include<string>
#include<list>
#include<utility>
#include<algorithm>
#include<iterator>

#include"GrammarFileReader.h"
#include"LL1Preprocess.h"
#include"C:\Users\Miracle\Source\Repos\CompilePrinciple\Automaton\Vlpp.h" // include files over projects

namespace hscp {
	struct LRState;
	struct LRTransition;
	template<typename TState>
	struct LROperation
	{
		enum type
		{
			S, R, N, ACC
		} OpType;
		TState* sid;
		int pid;

		/*LROperation(type t, TState* state_ptr, int prod_id): OpType(t),sid(state_ptr),pid(prod_id) {

		}*/
	};

	struct LRState
	{
		std::set<std::pair<std::string, std::list<std::string>>> projects;
		std::set<std::pair<std::string, std::list<std::string>>> start;
		std::set<std::pair<std::string, std::list<std::string>>> closure;

		std::vector<LRTransition*> trans;
	};
	struct LRTransition
	{
		std::string symbol;

		LRState* from, * to;
	};
	class LR0Automaton {
		template<typename TState>
		friend class Analyzer;
	private:
		std::vector<vl::Ptr<LRState>> states;
		std::vector<vl::Ptr<LRTransition>> transitions;

		std::vector<std::pair<std::string, std::list<std::string>>> productions;
		std::set<std::string> terminals;
		GrammarLoader& grammarloader;

		LRState* NewState(const std::set<std::pair<std::string, std::list<std::string>>>& prods) {
			std::set<std::pair<std::string, std::list<std::string>>> closed;
			for (const auto& p : prods) {
				auto ns = ProdClosure(p);
				std::copy(ns.begin(), ns.end(), std::inserter(closed, closed.begin()));
			}

			LRState* t = new LRState{ {}, prods, closed,{} };
			t->projects = prods;
			std::copy(closed.begin(), closed.end(), std::inserter(t->projects, t->projects.begin()));

			states.push_back(t);
			return t;
		}
		LRTransition* NewTransition(LRState* from, LRState* to, std::string symbol) {
			LRTransition* t = new LRTransition{ symbol,from,to };
			from->trans.push_back(t);

			transitions.push_back(t);
			return t;
		}
		std::set<std::pair<std::string, std::list<std::string>>> ProdClosure(const std::pair<std::string, std::list<std::string>>& prod) {
			std::set<std::pair<std::string, std::list<std::string>>> closeset;
			auto it = std::find(prod.second.begin(), prod.second.end(), ".");
			if (it == prod.second.end()) {
				throw "error production";
			}
			it++;
			if (it == prod.second.end() || (*it)[0] == '^') {
				return {};
			}
			else {
				auto bg = std::find_if(productions.begin(), productions.end(), [it](auto e) {return e.first == *it; });
				while (bg != productions.end() && bg->first == *it) {
					auto np = *bg;
					auto s = np.second.front();
					np.second.push_front(".");
					closeset.insert(np);
					if (s != *it)
					{
						auto nc = ProdClosure(np);
						std::copy(nc.begin(), nc.end(), std::inserter(closeset, closeset.begin()));
					}
					++bg;
				}
			}

			return std::move(closeset);
		}
		std::set<std::pair<std::string, std::pair<std::string, std::list<std::string>>>> getNext(LRState* state) {
			std::set<std::pair<std::string, std::pair<std::string, std::list<std::string>>>> ns;
			for (const auto& i : state->projects) {
				auto it = std::find(i.second.begin(), i.second.end(), ".");
				if (it == i.second.end() || (++it) == i.second.end()) {
					continue;
				}
				else {
					ns.insert(std::make_pair(*it, i));
				}
			}

			return std::move(ns);
		}
		void moveNexts(LRState* from) {
			auto nexts = getNext(from);
			if (nexts.size() == 0) return;
			auto cur = nexts.begin()->first;
			std::vector<std::pair<std::string, std::list<std::string>>> ps;
			for (const auto& n : nexts) {
				if (cur != n.first) {

					for (auto& p : ps) {
						auto it = find(p.second.begin(), p.second.end(), ".");
						auto i = it;
						it++; it++;
						p.second.erase(i);
						p.second.insert(it, ".");
					}
					std::set<std::pair<std::string, std::list<std::string>>> ts;
					std::copy(ps.begin(), ps.end(), std::inserter(ts, ts.begin()));
					std::set<std::pair<std::string, std::list<std::string>>> closed;
					for (const auto& p : ts) {
						auto ns = ProdClosure(p);
						std::copy(ns.begin(), ns.end(), std::inserter(closed, closed.begin()));
					}
					auto it = std::find_if(states.begin(), states.end(), [ts, closed](auto e) { return e->start == ts && e->closure == closed; });
					LRState* to;
					if (it == states.end()) {
						to = NewState(ts);
						moveNexts(to);
					}
					else to = it->Obj();
					NewTransition(from, to, cur);

					cur = n.first;
					ps.clear();
				}

				ps.push_back(n.second);
			}
			if (ps.size() > 0) {
				for (auto& p : ps) {
					auto it = find(p.second.begin(), p.second.end(), ".");
					auto i = it;
					it++; it++;
					p.second.erase(i);
					p.second.insert(it, ".");
				}
				std::set<std::pair<std::string, std::list<std::string>>> ts;
				std::copy(ps.begin(), ps.end(), std::inserter(ts, ts.begin()));
				std::set<std::pair<std::string, std::list<std::string>>> closed;
				for (const auto& p : ts) {
					auto ns = ProdClosure(p);
					std::copy(ns.begin(), ns.end(), std::inserter(closed, closed.begin()));
				}
				auto it = std::find_if(states.begin(), states.end(), [ts, closed](auto e) { return e->start == ts && e->closure == closed; });
				LRState* to;
				if (it == states.end()) {
					to = NewState(ts);
					moveNexts(to);
				}
				else to = it->Obj();
				NewTransition(from, to, cur);
			}

		}

		LR0Automaton(GrammarLoader& ld) :grammarloader(ld) {

		}
	public:
		static LR0Automaton Build(hscp::GrammarLoader& ld) {
			LR0Automaton at(ld);
			at.terminals.insert("^#");
			for (const auto& ps : ld.GetProductions()) {
				for (const auto& p : ps.second) {
					at.productions.push_back(std::make_pair(ps.first, p));

					for (const auto& s : p) {
						if (s[0] == '^')
							at.terminals.insert(s);
					}
				}
			}
			auto sp = at.productions[0];
			sp.second.push_front(".");
			auto nstate = at.NewState({ sp });

			at.moveNexts(nstate);

			return std::move(at);
		}

		std::map<LRState*, std::map<std::string, LROperation<LRState>>> LR0Table() {
			std::map<LRState*, std::map<std::string, LROperation<LRState>>> table;
			for (const auto& s : states) {
				for (const auto& t : s->trans) {
					if (t->symbol[0] == '^') {
						table[s.Obj()][t->symbol] = LROperation<LRState>{ LROperation<LRState>::S,t->to,-1 };
					}
					else {
						table[s.Obj()][t->symbol] = LROperation<LRState>{ LROperation<LRState>::N,t->to,-1 };
					}
				}

				for (const auto& p : s->projects) {

					if (p.second.back() == ".") {
						if (p.first[0] == '$') {
							table[s.Obj()]["^#"] = LROperation<LRState>{ LROperation<LRState>::ACC,nullptr,-1 };
							continue;
						}

						auto t = p.second;
						t.pop_back();
						int id = 0;
						for (int i = 0; i < productions.size(); i++)
						{
							if (productions[i].second == t) {
								id = i;
								break;
							}
						}

						for (const auto& t : terminals) {
							table[s.Obj()][t] = LROperation<LRState>{ LROperation<LRState>::R,nullptr,id };
						}
					}
				}
			}
			return std::move(table);
		}

		std::map<LRState*, std::map<std::string, LROperation<LRState>>> SLR1Table() {
			auto first = GetFirst(grammarloader);
			auto follow = GetFollow(grammarloader, first);
			std::map<LRState*, std::map<std::string, LROperation<LRState>>> table;

			for (const auto& s : states) {
				for (const auto& t : s->trans) {
					if (t->symbol[0] == '^') {
						table[s.Obj()][t->symbol] = LROperation<LRState>{ LROperation<LRState>::S,t->to,-1 };
					}
					else {
						table[s.Obj()][t->symbol] = LROperation<LRState>{ LROperation<LRState>::N,t->to,-1 };
					}
				}

				for (const auto& p : s->projects) {

					if (p.second.back() == ".") {
						if (p.first[0] == '$') {
							table[s.Obj()]["^#"] = LROperation<LRState>{ LROperation<LRState>::ACC,nullptr,-1 };
							continue;
						}

						auto t = p.second;
						t.pop_back();
						int id = 0;
						for (int i = 0; i < productions.size(); i++)
						{
							if (productions[i].second == t) {
								id = i;
								break;
							}
						}

						for (const auto& t : follow[p.first]) {
							table[s.Obj()][t] = LROperation<LRState>{ LROperation<LRState>::R,nullptr,id };
						}
					}
				}
			}

			return table;
		}
	};

	struct LR1State;
	struct LR1Transition;
	struct LR1State
	{
		std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> projects;
		std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> start;
		std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> closure;

		std::vector<LR1Transition*> trans;
	};
	struct LR1Transition
	{
		std::string symbol;

		LR1State* from, * to;
	};
	class LR1Automaton {
		template<typename TState>
		friend class Analyzer;
	private:
		std::vector<vl::Ptr<LR1State>> states;
		std::vector<vl::Ptr<LR1Transition>> transitions;

		std::vector<std::pair<std::string, std::list<std::string>>> productions;
		std::set<std::string> terminals;
		std::map<std::string, std::set<std::string>> firstset;
		GrammarLoader& grammarloader;


		LR1State* NewState(const std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>>& prods) {
			std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> closed;
			for (const auto& p : prods) {
				ProdClosure(p, closed);
				//std::copy(ns.begin(), ns.end(), std::inserter(closed, closed.begin()));
			}

			LR1State* t = new LR1State{ {}, prods, closed,{} };
			t->projects = prods;
			std::copy(closed.begin(), closed.end(), std::inserter(t->projects, t->projects.begin()));

			states.push_back(t);
			return t;
		}
		LR1Transition* NewTransition(LR1State* from, LR1State* to, std::string symbol) {
			LR1Transition* t = new LR1Transition{ symbol,from,to };
			from->trans.push_back(t);

			transitions.push_back(t);
			return t;
		}
		// <A->PQR...,{s}>
		// recursively get closure of a project in state
		void ProdClosure(
			const std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>& prod,
			std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>>& cset
		) {

			//std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> closeset;
			auto it = std::find(prod.first.second.begin(), prod.first.second.end(), "."); // find "."
			if (it == prod.first.second.end()) {
				throw "error production";
			}
			it++; // move to symbol after "."
			if (it == prod.first.second.end() || (*it)[0] == '^') { // no need for reduce proj or followed by terminal
				return;
			}
			else {
				// find productions for one nonterminal
				auto bg = std::find_if(productions.begin(), productions.end(), [it](auto e) {return e.first == *it; });
				while (bg != productions.end() && bg->first == *it) {
					auto np = *bg; // copy production
					auto s = np.second.front(); // symbol to close
					np.second.push_front("."); // start this production

					auto seq = prod.first.second;
					seq.push_back(*prod.second.begin());
					auto seqbegin = std::find(seq.begin(), seq.end(), ".");
					seqbegin++; seqbegin++;
					auto seqfirst = getFirst(seqbegin, seq.end(), firstset); // get look ahead set for a closure production

					for (const auto& f : seqfirst) {
						auto size = cset.size();
						auto insed = cset.insert(std::make_pair(np, std::set<std::string>({ f })));
						if (size<cset.size())
						{
							ProdClosure(*(insed.first), cset);
							//std::copy(nc.begin(), nc.end(), std::inserter(closeset, closeset.begin()));
						}
					}
					++bg;
					
				}
			}

			return;
		}
		std::set<std::pair<std::string, std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>>> getNext(LR1State* state) {
			std::set<std::pair<std::string, std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>>> ns;
			for (const auto& i : state->projects) {
				auto it = std::find(i.first.second.begin(), i.first.second.end(), ".");
				if (it == i.first.second.end() || (++it) == i.first.second.end()) {
					continue;
				}
				else {
					ns.insert(std::make_pair(*it, std::make_pair(i.first, i.second)));
				}
			}

			return std::move(ns);
		}
		void moveNexts(LR1State* from) {
			auto nexts = getNext(from);
			if (nexts.size() == 0) return;
			auto cur = nexts.begin()->first;
			std::vector<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> ps;
			for (const auto& n : nexts) {
				if (cur != n.first) {

					for (auto& p : ps) {
						auto it = find(p.first.second.begin(), p.first.second.end(), ".");
						auto i = it;
						it++; it++;
						p.first.second.erase(i);
						p.first.second.insert(it, ".");
					}
					std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> ts;
					std::copy(ps.begin(), ps.end(), std::inserter(ts, ts.begin()));
					std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> closed;
					for (const auto& p : ts) {
						ProdClosure(p, closed);
						//std::copy(ns.begin(), ns.end(), std::inserter(closed, closed.begin()));
					}
					auto it = std::find_if(states.begin(), states.end(), [ts, closed](auto e) { return e->start == ts && e->closure == closed; });
					LR1State* to;
					if (it == states.end()) {
						to = NewState(ts);
						moveNexts(to);
					}
					else to = it->Obj();
					NewTransition(from, to, cur);

					cur = n.first;
					ps.clear();
				}

				ps.push_back(n.second);
			}
			if (ps.size() > 0) {
				for (auto& p : ps) {
					auto it = find(p.first.second.begin(), p.first.second.end(), ".");
					auto i = it;
					it++; it++;
					p.first.second.erase(i);
					p.first.second.insert(it, ".");
				}
				std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> ts;
				std::copy(ps.begin(), ps.end(), std::inserter(ts, ts.begin()));
				std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> closed;
				for (const auto& p : ts) {
					ProdClosure(p, closed);
					//std::copy(ns.begin(), ns.end(), std::inserter(closed, closed.begin()));
				}
				auto it = std::find_if(states.begin(), states.end(), [ts, closed](auto e) { return e->start == ts && e->closure == closed; });
				LR1State* to;
				if (it == states.end()) {
					to = NewState(ts);
					moveNexts(to);
				}
				else to = it->Obj();
				NewTransition(from, to, cur);
			}

		}
		LR1Automaton(GrammarLoader& ld) :grammarloader(ld) {
			firstset = GetFirst(ld);
		}
	public:
		static LR1Automaton Build(GrammarLoader& ld) {
			LR1Automaton at(ld);
			at.terminals.insert("^#");
			for (const auto& ps : ld.GetProductions()) {
				for (const auto& p : ps.second) {
					at.productions.push_back(std::make_pair(ps.first, p));

					for (const auto& s : p) {
						if (s[0] == '^')
							at.terminals.insert(s);
					}
				}
			}

			auto sp = at.productions[0];
			sp.second.push_front(".");
			auto nstate = at.NewState({ std::make_pair(sp, std::set<std::string>({"^#"})) });

			at.moveNexts(nstate);

			return std::move(at);
		}

		void MergeLALR1() {
			for (auto& s : states) {
				std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> ts;
				//std::copy(s->start.begin(), s->start.end(), std::back_inserter(ts));
				auto cur = (*s->start.begin()).first;
				if (s->start.size() > 1) {
					std::set<std::string> ss;
					for (const auto& t : s->start) {
						if (cur != t.first) {
							ts.insert(std::make_pair(cur, ss));
							ss.clear();
							cur = t.first;
						}
						std::copy(t.second.begin(), t.second.end(), std::inserter(ss, ss.begin()));
					}
					if (ss.size() > 0) {
						ts.insert(std::make_pair(cur, ss));
						ss.clear();
					}
					s->start = ts;
				}
				ts.clear();


				if (s->closure.size() > 1) {
					cur = (*s->closure.begin()).first;
					std::set<std::string> ss;
					for (const auto& t : s->closure) {
						if (cur != t.first) {
							ts.insert(std::make_pair(cur, ss));
							ss.clear();
							cur = t.first;
						}
						std::copy(t.second.begin(), t.second.end(), std::inserter(ss, ss.begin()));
					}
					if (ss.size() > 0) {
						ts.insert(std::make_pair(cur, ss));
						ss.clear();
					}
					s->closure = ts;
				}

				s->projects.clear();

				std::copy(s->start.begin(), s->start.end(), std::inserter(s->projects, s->projects.begin()));
				std::copy(s->closure.begin(), s->closure.end(), std::inserter(s->projects, s->projects.begin()));
			}
			/*std::vector<decltype(states.begin())> removeit;
			for (auto i = states.begin(); i != states.end()-1; ++i)
			{
				for (auto j = i + 1; j != states.end(); j++)
				{
					if (i->Obj()->projects.size() == j->Obj()->projects.size()) {
						std::vector<std::pair<std::string, std::list<std::string>>> t1,t2;
						std::vector<std::set<std::string>> ss;
						for (const auto& p : i->Obj()->projects) {
							t1.push_back(p.first);
							ss.push_back(p.second);
						}
						int index = 0;
						for (const auto& p : j->Obj()->projects) {
							t2.push_back(p.first);
							std::copy(p.second.begin(), p.second.end(), std::inserter(ss[index], ss[index].begin()));
							index++;
						}
						if (t1 == t2) {
							std::set<std::pair<std::pair<std::string, std::list<std::string>>, std::set<std::string>>> mp;
							for (auto i = 0; i < t1.size(); i++) {
								mp.insert(std::make_pair(t1[i], ss[i]));
							}
							j->Obj()->projects = mp;
							removeit.push_back(i);
							for (auto& t : transitions) {
								if (t.Obj()->to == i->Obj()) {
									t.Obj()->to = j->Obj();
								}
							}
						}
					}
				}

			}
			for (auto& r : removeit) {
				states.erase(r);
			}*/
		}
		std::map<LR1State*, std::map<std::string, LROperation<LR1State>>> LR1Table() {
			std::map<LR1State*, std::map<std::string, LROperation<LR1State>>> table;

			for (const auto& s : states) {
				for (const auto& t : s->trans) {
					if (t->symbol[0] == '^') {
						table[s.Obj()][t->symbol] = LROperation<LR1State>{ LROperation<LR1State>::S,t->to,-1 };
					}
					else {
						table[s.Obj()][t->symbol] = LROperation<LR1State>{ LROperation<LR1State>::N,t->to,-1 };
					}
				}

				for (const auto& p : s->projects) {

					if (p.first.second.back() == ".") {
						if (p.first.first[0] == '$') {
							table[s.Obj()]["^#"] = LROperation<LR1State>{ LROperation<LR1State>::ACC,nullptr,-1 };
							continue;
						}

						auto t = p.first.second;
						t.pop_back();
						int id = 0;
						for (int i = 0; i < productions.size(); i++)
						{
							if (productions[i].second == t) {
								id = i;
								break;
							}
						}

						for (const auto& t : p.second) {
							table[s.Obj()][t] = LROperation<LR1State>{ LROperation<LR1State>::R,nullptr,id };
						}
					}
				}
			}

			return std::move(table);
		}
	};
}