#pragma once
#include<map>
#include<string>
#include<list>
#include<vector>
#include<stack>

#include"LL1Preprocess.h"
#include"LexMatcher.h" // include files over projects

namespace {
	void PrintStack(const std::deque<std::string>& st) {
		
		for (const auto& t : st) {
			std::cout << t << ' ';
		}
		std::cout << '\n';
	}
}
namespace hscp {
	class Analyzer {
	private:
		const std::map<std::string, std::map<std::string, std::list<std::string>>>& table;
		const std::vector<Token>& tokenstream;

		std::vector<Token> errors;
	public:
		Analyzer(const std::map<std::string, std::map<std::string, std::list<std::string>>>& table, const std::vector<Token>& tokenstream):table(table),tokenstream(tokenstream) {
			std::deque<std::string> ana;
			ana.push_back("^#");
			ana.push_back(GRAMMAR_START_SYMBOL);
			PrintStack(ana);
			for (auto i = tokenstream.begin(); i != tokenstream.end();) {
				if (ana.back() == ('^' + i->is)) {
					if (i->is == "#")
						break;
					else {
						ana.pop_back();
						PrintStack(ana);
						++i;
					}
				}else
				if (table.at(ana.back()).find('^' + i->is) != table.at(ana.back()).end()) {
					auto S = ana.back();
					ana.pop_back();
					for (auto j = table.at(S).at('^' + i->is).rbegin(); j != table.at(S).at('^' + i->is).rend(); ++j)
					{
						if (*j != "^Epsilon")
						{
							ana.push_back(*j);
						}
						PrintStack(ana);
						
					}

				}
				else {
					errors.push_back(*i);
					errors.back().type = "Err";
					++i;
				}
			}
		}

		std::vector<Token>& GetErrors() {
			return errors;
		}
		void PrintErrors() {
			if (errors.size() == 0) {
				std::cout << "No Err Detected.\n";
			}
			for (const auto& e : errors) {
				std::cout << "Error: " << e.content << " (" << e.is << ") " << "at line: " << e.line << " ,column: " << e.column << ".\n";
			}
		}
	};
}
