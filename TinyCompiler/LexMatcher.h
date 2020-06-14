#pragma once
#include<string>
#include<fstream>

#include"Automaton.h"

namespace hscp {
	struct Token { // an lexical token
		std::string type; // nonused token type
		std::string is; // tells token name
		std::string content; // token contents
		long long value; // nonused token contents
		int line, column; // token position in source file
	};
	class Matcher {
	private:
		// read a token from file stream
		bool match(std::istream& ist, Token& token, int& line, int& column) {
			state* current = at.startState;  // match from start
			std::string word;
			char ch;
			auto it_t = current->trans.begin();
			decltype(it_t) last_match;
			int tl = -1, tc = -1;

			while (ist.get(ch)) {

				if (ch == '\n' || ch == '\r') { // count lines and column
					line++;
					column = 0;
				}
				else column++;

				if ((it_t = find_if(current->trans.begin(), current->trans.end(), [ch](transition* t) {return t->input == ch; })) != current->trans.end()) {
					// if matched
					if (tl == tc && tl == -1) { // record first character position
						tl = line; tc = column;
					}

					word += ch;
					last_match = it_t;
					current = (*it_t)->to; // move next
				}
				else if (word == "" && (ch == ' ' || ch == '\n' || ch == '\t'))continue; // first space char
				else if (current->is == "numberval" && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')) goto err; // number before alphbets
				else break; // no matched character
			}

			if (!current->finalState) { // match not finished
				token = { "","Err",word,0,tl,tc };
				return true;
			}
			else {
				if (!(ch == ' ' || ch == '\n' || ch == '\t'))
					ist.putback(ch); // put back a non-space char
				token = { "",current->is,word,0,tl,tc };
				return true;
			}
		err:
			do // read all alphbets behind
			{
				word += ch;
			} while (ist.get(ch) && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_'));
			ist.putback(ch);
			token = { "","Err",word,0,tl,tc };
			return true;
		}
	public:
		Automaton at;

		Matcher(const Automaton& automaton) :at(automaton) {}
		// read given file
		std::vector<Token> ReadFile(const std::string& route) {
			if (!std::filesystem::exists(route)) {
				std::cout << "\nsource file not exists\n";
				return {};
			}
			std::vector<Token> tokens;
			std::ifstream fin(route);
			Token t;
			int line = 1, column = 0;
			while (fin.peek() != EOF)
			{
				if (match(fin, t, line, column)) {
					tokens.push_back(t);
				}
			}


			return tokens;
		}
	};
	// print tokens
	void PrintTokens(std::vector<Token>& tokens) {
		for (const auto& t : tokens) {
			if (t.is == "Err")
				std::cout << "Err: unacceptable character sequence \"" << t.content << "\",\n at: line " << t.line << " column " << t.column << "\n";
			else std::cout << "Name: " << t.is << "\n Content: " << t.content << ",\n at: line " << t.line << " column " << t.column << "\n";
		}
	}
}