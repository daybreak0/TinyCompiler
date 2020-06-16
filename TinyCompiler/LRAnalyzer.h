#pragma once


#include"LRAutos.h"
#include"LexMatcher.h" // include files over projects

namespace {
	void PrintStack(const std::deque<hscp::Token>& st) {

		for (const auto& t : st) {
			std::cout << t.is << ' ';
		}
		std::cout << '\n';
	}
}
namespace hscp {
	// abstract syntax tree node
	struct ASTNode
	{
		Token token;
		std::vector<ASTNode*> children;

		~ASTNode()
		{
			for (auto& c : children) {
				delete c;
				c = nullptr;
			}
		}
	};
	// abstract syntax tree
	struct AST
	{
		ASTNode* root;

		~AST()
		{
			delete root;
			root = nullptr;
		}
	};
	// abstract syntax tree node
	struct AnalyzeTreeNode
	{
		Token token;
		std::string symbol;
		std::list<AnalyzeTreeNode*> children;

		void Destroy() {
			for (auto& c : children) {
				delete c;
				c = nullptr;
			}
		}
		~AnalyzeTreeNode()
		{
			
		}
	};
	// abstract syntax tree
	struct AnalyzeTree
	{
		AnalyzeTreeNode* root;

		void Destroy() {
			delete root;
			root = nullptr;
		}
		~AnalyzeTree()
		{
			
		}
	};
	// Analyzer support for all LR method
	template<typename TState>
	class Analyzer {
	private:
		const std::map<TState*, std::map<std::string, LROperation<TState>>>& table;
		const std::vector<std::pair<std::string, std::list<std::string>>>& productions;
		const std::vector<Token>& tokenstream;

		AnalyzeTree tree;
		std::vector<Token> errors;
	public:
		/*Analyzer(const LR0Automaton& at, const std::map<TState*, std::map<std::string, LROperation<TState>>>& table, const std::vector<Token>& tokenstream) :table(table), productions(at.productions), tokenstream(tokenstream) {
			std::deque<Token> symbol;
			std::deque<TState*> state;

			state.push_back(at.states[0].Obj());

			for (auto i = tokenstream.begin(); i != tokenstream.end();) {
				auto it = find(state.back()->trans.begin(), state.back()->trans.end(),(void*)0);
				int pn;
				if (table.at(state.back()).find('^' + i->is) == table.at(state.back()).end()) {
					errors.push_back(*i);
					i++;
				}
				switch (table.at(state.back()).at('^'+i->is).OpType)
				{
				case LROperation<TState>::ACC:
					return;
				case LROperation<TState>::S:
					state.push_back(table.at(state.back()).at('^' + i->is).sid);
					symbol.push_back(*i);
					++i;
					PrintStack(symbol);
					break;
				case LROperation<TState>::R:
					pn = table.at(state.back()).at('^' + i->is).pid;
					for (int n = 0; n < productions[pn].second.size(); n++) {
						symbol.pop_back();
						state.pop_back();
					}
					it = find_if(state.back()->trans.begin(), state.back()->trans.end(), [t = productions[pn]](auto e){return e->symbol == t.first; });
					state.push_back((*it)->to);
					symbol.push_back({"", productions[pn].first });
					PrintStack(symbol);
					break;
				case LROperation<TState>::N:
				default:
					break;
				}
			}
		}*/
		Analyzer(const LR1Automaton& at, const std::map<TState*, std::map<std::string, LROperation<TState>>>& table, const std::vector<Token>& tokenstream) :table(table), productions(at.productions), tokenstream(tokenstream) {
			std::deque<Token> symbol_stack;
			std::deque<TState*> state;
			std::deque<AnalyzeTreeNode*> syntax;

			state.push_back(at.states[0].Obj());
			AnalyzeTreeNode* snode = nullptr;
			for (auto i = tokenstream.begin(); i != tokenstream.end();) {
				auto it = find(state.back()->trans.begin(), state.back()->trans.end(), (void*)0);
				int pn;
				if (table.at(state.back()).find('^' + i->is) == table.at(state.back()).end()) {
					errors.push_back(*i);
					i++;
					continue;
				}
				switch (table.at(state.back()).at('^' + i->is).OpType)
				{
				case LROperation<TState>::ACC:
					tree.root = syntax.back();
					return;
				case LROperation<TState>::S:
					state.push_back(table.at(state.back()).at('^' + i->is).sid);
					symbol_stack.push_back(*i);
					syntax.push_back(new AnalyzeTreeNode{ *i, i->is, {} });
					++i;
					PrintStack(symbol_stack);
					break;
				case LROperation<TState>::R:
					pn = table.at(state.back()).at('^' + i->is).pid;
					snode = new AnalyzeTreeNode{ {},productions[pn].first,{} };
					for (int n = 0; n < productions[pn].second.size(); n++) {
						snode->children.push_front(syntax.back());
						syntax.pop_back();
						symbol_stack.pop_back();
						state.pop_back();
					}
					it = find_if(state.back()->trans.begin(), state.back()->trans.end(), [t = productions[pn]](auto e){return e->symbol == t.first; });
					state.push_back((*it)->to);
					symbol_stack.push_back({ "", productions[pn].first });
					syntax.push_back(snode);
					PrintStack(symbol_stack);
					break;
				case LROperation<TState>::N:
				default:
					break;
				}
			}

			tree.root = syntax.back();
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

		AnalyzeTree& GetAnalyzeTree() {
			return tree;
		}
	};
}