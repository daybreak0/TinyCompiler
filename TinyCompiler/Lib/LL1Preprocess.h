#pragma once
#include"GrammarFileReader.h"
#include<algorithm>
#include<iterator>
#include<vector>
namespace {
	// for set containing iterator
	typedef std::set<std::list<std::string>>::iterator iter;
	struct ite_cmp
	{
		template<typename I>
		bool operator()(const I& lh, const I& rh) const { return *lh < *rh; }
	};

	// tell if a symbol can derive to epsilon
	bool deriveToEpsilon(const std::map<std::string, std::set<std::list<std::string>>>& productions, const std::string& symbol) {
		if (symbol == "^Epsilon") return true;
		else if (symbol[0] == '^') return false;
		else {
			for (const auto& sp : productions.at(symbol)) {
				bool pe = true;
				for (const auto& s : sp) {
					if (symbol != s && !deriveToEpsilon(productions, s)) {
						pe = false;
						break;
					}
				}
				if (pe)
					return true;
				continue;
			}
			return false;
		}
	}
	bool deriveToEpsilon(std::list<std::string>::const_iterator symbol_begin, std::list<std::string>::const_iterator symbol_end, const std::map<std::string, std::set<std::list<std::string>>>& productions) {
		for (auto i = symbol_begin; i != symbol_end; ++i) {
			if (!deriveToEpsilon(productions, *i))
				return false;
		}
		return true;
	}
	// get first set of a symbol sequence
	std::set<std::string> getFirst(std::list<std::string>::const_iterator symbol_begin, std::list<std::string>::const_iterator symbol_end, const std::map<std::string, std::set<std::string>>& firstset) {
		std::set<std::string> fset;
		for (auto i = symbol_begin; i != symbol_end; ++i) {
			if (*i == ".") continue;
			if ((*i)[0] == '^') {
				if (*i == "^Epsilon") {
					continue;
				}
				else {
					fset.insert(*i);
					goto directend;
				}
			}
			else {
				std::copy(firstset.at(*i).begin(), firstset.at(*i).end(), std::inserter(fset, fset.begin()));
				if (firstset.at(*i).find("^Epsilon") == firstset.at(*i).end()) {
					goto directend;
				}
				else {
					fset.erase("^Epsilon");
				}
			}

		}

		fset.insert("^Epsilon");
	directend:
		return std::move(fset);
	}

	// get select sets for a symbol
	std::vector<std::set<std::string>> getSelect(
		const std::map<std::string, std::set<std::list<std::string>>>& productions,
		const std::string& symbol,
		const std::map<std::string, std::set<std::string>>& firstset,
		const std::map<std::string, std::set<std::string>>& followset,
		std::map<std::string, bool>& record
	) {
		std::set<std::string> select;
		std::vector<std::set<std::string>> selects;
		for (const auto& p : productions.at(symbol)) {
			bool toE = deriveToEpsilon(p.begin(), p.end(), productions);

			for (const auto& s : p) {
				//toE |= deriveToEpsilon(s, s, record);

				if (s[0] == '^')
				{
					select.insert(s);
					break;
				}
				else
					std::copy(firstset.at(s).begin(), firstset.at(s).end(), std::inserter(select, select.begin()));
			}
			if (p.front() != "^Epsilon")
			{
				select.erase("^Epsilon");
				if (toE) {
					std::copy(followset.at(symbol).begin(), followset.at(symbol).end(), std::inserter(select, select.begin()));
				}
			}


			selects.push_back(select);
			select.clear();
		}

		return std::move(selects);
	}
}

namespace hscp {

	// declare
	void PrintSet(const std::string& settype, const std::map<std::string, std::set<std::string>>& firstset);

	// remove left recursion in grammar production
	void RemoveLeftRecursion(GrammarLoader& loader) {
		auto& ps = loader.GetProductions();


		for (auto i = ps.begin(); i != ps.end(); ++i) {
			for (auto j = ps.begin(); j != i; j++) // replace previously processed
			{
				//if (j->first.back() != '\'') continue;
				decltype(i->second) ts;
				for (auto& p : i->second) {
					std::list<std::string> tls;
					if (p.front() == j->first) {
						auto t = p;
						t.pop_front();
						for (const auto& p2 : j->second) {
							std::copy(p2.begin(), p2.end(), std::back_inserter(tls));
							std::copy(t.begin(), t.end(), std::back_inserter(tls));
							ts.insert(tls);
							tls.clear();
						}
					}
					else {
						ts.emplace(p);
					}
				}
				i->second = ts;
			}

			// remove immediate left recursion
			std::set<std::list<std::string>> newAi;
			std::set<std::list<std::string>> Aiq;
			for (auto& p : i->second) {
				if (p.front()[0] != '^' && p.front() == i->first) { // contains recursion
					auto t = p;
					t.push_back(i->first + '\'');
					t.pop_front();

					Aiq.insert(t);
				}
				else { // not recursive
					auto t = p;
					t.push_back(i->first + '\'');

					newAi.insert(t);
				}
			}
			if (Aiq.size() != 0) {
				Aiq.insert({ "^Epsilon" });

				i->second = newAi;
				ps.emplace(i->first + '\'', Aiq);
			}
		}
	}
	// extract left common factor in grammar production
	void ExtractLeftCommonFactor(GrammarLoader& loader) {
		auto& ps = loader.GetProductions();

		for (auto i = ps.begin(); i != ps.end(); ++i) {
			int count = 0;
			int cnum = 0;
			std::string last = i->second.begin()->front();

			std::set<iter, ite_cmp> its;
			for (auto f = i->second.begin(); f != i->second.end(); ++f) { // for each parallel production
				if (f->front() == last) // has common factor (not the first time)
				{
					count++;
					its.insert(f);
				}
				else {
					if (count > 1) { // has common factor
						for (const auto& it : its) {
							auto t = *(it);
							t.pop_front();
							if (t.size() < 1) t.push_back("^Epsilon"); // if has no symbol except common factor
							ps[i->first + '`' + std::to_string(cnum)].emplace(t);
							i->second.erase(it);
						}
						i->second.insert({ last,i->first + '`' + std::to_string(cnum) });

						cnum++;
					}

					count = 1;
					last = f->front();
					its.clear();
					its.insert(f);
				}
			}
			if (count > 1) { // the last common factor
				for (const auto& it : its) {
					auto t = *(it);
					t.pop_front();
					if (t.size() < 1) t.push_back("^Epsilon"); // if has no symbol except common factor
					ps[i->first + '`' + std::to_string(cnum)].emplace(t);
					i->second.erase(it);
				}
				i->second.insert({ last,i->first + '`' + std::to_string(cnum) });

				cnum++;
			}
		}
	}
	// get first set for each symbol
	std::map<std::string, std::set<std::string>> GetFirst(GrammarLoader& loader) {
		const auto& ps = loader.GetProductions();
		std::map<std::string, std::set<std::string>> firstset;
		std::map<std::string, std::set<std::string>> direct;
		std::map<std::string, std::set<std::string>> indirect;

		bool added = true;
		while (added)
		{
			added = false;

			for (const auto& p : ps) { // each production group
				for (const auto& i : p.second) { // each single production
					bool pe = true;
					std::set<std::string> before = firstset[p.first];
					for (auto s = i.begin(); s != i.end(); ++s) {
						auto t = s; t++;
						if (!deriveToEpsilon(ps, *s)) {
							pe = false;
							if ((*s)[0] == '^') {
								firstset[p.first].insert(*s);
								firstset[*s].insert(*s);
							}
							else {
								std::copy(firstset[*s].begin(), firstset[*s].end(), std::inserter(firstset[p.first], firstset[p.first].begin()));
							}
							break;
						}
						else {
							if (*s == "^Epsilon") {
								firstset[p.first].insert(*s);
							}
							else {
								std::set<std::string> t = firstset[*s];
								t.erase("^Epsilon");
								std::copy(t.begin(), t.end(), std::inserter(firstset[p.first], firstset[p.first].begin()));

							}
						}
					}
					if (pe)
						firstset[p.first].insert("^Epsilon");
					if (before != firstset[p.first])
						added = true;

				}
			}
		}


		return std::move(firstset);
	}
	// get follow set for each symbol
	std::map<std::string, std::set<std::string>> GetFollow(GrammarLoader& loader, const std::map<std::string, std::set<std::string>>& firstset) {
		const auto& ps = loader.GetProductions();
		std::map<std::string, std::set<std::string>> followset, record;
		followset[GRAMMAR_START_SYMBOL].insert("^#"); // a start symbol's follow set contains delimiter
		record = followset;
		//for (const auto& s : ps) { // get each
		//	followset[s.first] = getFollow(ps, s.first, firstset, record);
		//}
		bool added = true;
		while (added)
		{
			added = false;

			for (const auto& s : ps) { // get each
				for (const auto& p : s.second) { // each production
					if (p.size() < 2) continue;
					auto bg = p.begin(); bg;
					auto ed = p.end(); ed--;

					for (auto i = bg; i != p.end(); ++i) {
						if ((*i)[0] == '^') continue;


						if (i == ed) {
							std::set<std::string> before = followset[*i];
							std::copy(followset[s.first].begin(), followset[s.first].end(), std::inserter(followset[*i], followset[*i].begin()));

							if (before != followset[*i]) {
								added = true;
							}
						}
						else {

							auto jbg = i; jbg++;
							std::set<std::string> tfollow;
							std::set<std::string> before = followset[*i];
							for (auto j = jbg; j != p.end(); ++j) {
								if ((*j)[0] == '^') {
									tfollow.insert(*j);
									break;
								}
								else {
									std::copy(firstset.at(*j).begin(), firstset.at(*j).end(), std::inserter(tfollow, tfollow.begin()));
								}
							}
							tfollow.erase("^Epsilon");
							if (deriveToEpsilon(jbg, p.end(), ps))
								std::copy(followset[s.first].begin(), followset[s.first].end(), std::inserter(tfollow, tfollow.begin()));

							std::copy(tfollow.begin(), tfollow.end(), std::inserter(followset[*i], followset[*i].begin()));

							if (followset[*i] != before) {

								added = true;
							}
						}

					}

				}
			}
		}


		return std::move(followset);
	}
	// tell whether the grammar is LL(1) grammar
	bool IsLL_1(
		GrammarLoader& loader,
		const std::map<std::string, std::set<std::string>>& firstset,
		const std::map<std::string, std::set<std::string>>& followset
	) {
		const auto& ps = loader.GetProductions();
		for (const auto& p : ps) {
			for (const auto& l : p.second) {
				if (l.front() == p.first) // left recursion
					return false;
			}
		}
		std::set<std::string> ts;

		std::map<std::string, bool> record;

		std::map<std::string, std::set<std::string>> allSelect;
		for (const auto& p : ps) {
			std::vector<std::set<std::string>> selects = getSelect(ps, p.first, firstset, followset, record);
			auto iter = selects.begin();
			for (const auto& sp : p.second) {
				std::string pc = "";
				for (const auto& s : sp) {
					pc += (s + " ");
				}
				allSelect[p.first + " -> " + pc] = *iter; ++iter;
			}
			for (auto i = selects.begin(); i != selects.end(); ++i) {
				for (auto j = i; j != selects.end(); ++j) {
					if (j == i) continue;

					std::set_intersection(i->begin(), i->end(), j->begin(), j->end(), std::inserter(ts, ts.begin()));

					if (ts.size() > 0) {
						//PrintSet("Select", allSelect);

						std::cout << "INTERSECTION: ";
						for (const auto& s : ts) {
							std::cout << s << ' ';
						}
						std::cout << "\n";
						return false;
					}
				}
			}
		}

		//PrintSet("Select", allSelect);

		return true;
	}
	// build LL(1) analysis table
	std::map<std::string, std::map<std::string, std::list<std::string>>> LL_1Table(
		GrammarLoader& loader,
		const std::map<std::string, std::set<std::string>>& firstset,
		const std::map<std::string, std::set<std::string>>& followset
	) {
		const auto& ps = loader.GetProductions();
		std::map<std::string, std::map<std::string, std::list<std::string>>> analyze;
		for (const auto& pp : ps) {
			for (const auto& sp : pp.second) {
				auto fset = getFirst(sp.begin(), sp.end(), firstset);
				for (const auto& s : fset) {
					if (s == "^Epsilon") continue;
					analyze[pp.first][s] = sp;
				}

				if (deriveToEpsilon(sp.begin(), sp.end(), ps)) {
					for (const auto& s : followset.at(pp.first)) {
						analyze[pp.first][s] = sp;
					}
				}
			}
		}

		return std::move(analyze);
	}

	// print sets
	void PrintSet(const std::string& settype, const std::map<std::string, std::set<std::string>>& firstset) {
		for (const auto& f : firstset) {
			std::cout << settype << "( " << f.first << " ) = ";
			for (const auto& s : f.second) {
				std::cout << s << ' ';
			}
			std::cout << "\n";
		}
	}

}