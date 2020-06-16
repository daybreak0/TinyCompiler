#pragma once

#include<iostream>
#include<fstream>
#include<sstream>
#include<filesystem>
#include<string>
#include<map>
#include<set>
#include<vector>
#include<list>
namespace hscp {
	constexpr auto GRAMMAR_START_SYMBOL = "S";
	class GrammarLoader {
	private:
		std::ifstream fin; // file stream
		std::map<std::string, std::set<std::list<std::string>>> productions; // productions
		std::set<std::string> terminals; // save all terminals
		// print a production
		void print(const decltype(productions)::value_type& e) {
			std::cout << e.first << " -> ";
			bool f = true;
			for (const auto& t : e.second) {
				if (f)
					f = false;
				else std::cout << " | ";
				for (const auto& r : t) {
					std::cout << r << ' ';
				}
			}
			std::cout << '\n';
		}
		// from file load production
		void load() {
			std::string line;

			while (std::getline(fin, line)) {
				std::stringstream ss(line);
				std::string n, p;
				std::getline(ss, n, '-');
				ss.get();
				ss >> std::ws;

				std::list<std::string> ps;
				while (ss >> p) {
					if (p != "|")
					{
						if (p[0] == '^') {
							terminals.insert(p);
						}
						ps.push_back(p);
					}
					else {
						productions[n].emplace(ps);
						ps.clear();
					}
				}
				productions[n].emplace(ps);
				ps.clear();
			}

		}
		// check all productions containing nonterminals not appearing at left
		std::vector<std::pair<std::string, std::set<std::list<std::string>>>> check() {
			std::vector<std::pair<std::string, std::set<std::list<std::string>>>> e;
			for (const auto& p : productions) {
				for (const auto& t : p.second) {
					for (const auto& r : t) {
						if (r[0] == '^') // terminal hint
							continue;
						else {
							if (productions.find(r) != productions.end())
								continue;
							else
								e.emplace_back(p);
						}
					}
				}
			}
			return std::move(e);
		}
	public:
		GrammarLoader() {
#ifdef _DEBUG
			constexpr auto route = "Data\\grammar.txt";
#else
			constexpr auto route = "grammar.txt";
#endif

			if (!std::filesystem::exists(route)) {
				std::cout << "Grammar Definations not Found.\n";
			}
			fin.open(route, std::ios::in | std::ios::_Nocreate);

			load();
			ErrorCheck();
		}
		// print errors
		void ErrorCheck() {
			auto err = check();
			if (err.size() > 0) {
				std::cout << "\nError(s) found, isolated nonterminals contained:\n";
				for (const auto& e : err) {
					print(e);
				}
			}
			else
				std::cout << "\nNo Error Detected.\n";
		}
		// print production
		void Print() {
			for (const auto& p : productions) {
				print(p);
			}
		}

		//LR preprocess
		void EnableLR() {
			productions[std::string("$") + GRAMMAR_START_SYMBOL] = { {GRAMMAR_START_SYMBOL} };
		}

		decltype(productions)& GetProductions() {
			return productions;
		}
	};
}