#pragma once
#include <vector>
#include "LexMatcher.h"
#include "LRAnalyzer.h"
#include "SematicLoader.h"
#include <functional>
#include<iomanip>

namespace hscp {
	// abstract syntax tree node
	struct ASTNode
	{
		Token token;
		std::string op;
		std::string val;
		std::vector<ASTNode*> children;
		void Destroy() {
			for (auto& c : children) {
				c->Destroy();
				delete c;
				c = nullptr;
			}
		}
		~ASTNode()
		{
		}
	};
	// abstract syntax tree
	struct AST
	{
		ASTNode* root;
		void Destroy() {
			root->Destroy();
			delete root;
			root = nullptr;
		}
		~AST()
		{
		}
	};
	namespace {
		void traverse(ASTNode* node, int level) {
			std::cout << std::setw(level * 4) << node->op <<", val: "<<node->val << std::endl;

			for (const auto& n : node->children) {
				traverse(n, level + 1);
			}
		}

		
	}
	void PrintAST(AST ast) {
		traverse(ast.root, 1);
	}
	
	
	class SematicProcesser {
	private:
		AnalyzeTree& atree;
		const SematicLoader& loader;
		AST ast;

		std::set<std::string> symbol_table;

		// behavior
		std::map<std::string, std::function<ASTNode* (std::vector<std::string>, std::vector<ASTNode*>)>> actions = {
			{"Equal",
			[](std::vector<std::string> params, std::vector<ASTNode*> childrenNodes) {
				return childrenNodes[0];
			}},
			{"Append",
			[](std::vector<std::string> params, std::vector<ASTNode*> childrenNodes) {
				if (childrenNodes[0]->op == "sequence") {
					childrenNodes[0]->children.push_back(childrenNodes[1]);
					return childrenNodes[0];
				}
				else {
					auto t = new ASTNode{{}, "sequence", "", childrenNodes};
					return t;
				}
			}},
			{"Node",
			[](std::vector<std::string> params, std::vector<ASTNode*> childrenNodes) {
				ASTNode* t;
				if (params[0][0] >= 65 && params[0][0] <= 90)
					t = new ASTNode{{},params[0],"",childrenNodes};
				else
				{
					auto op_it = std::find_if(childrenNodes.begin(), childrenNodes.end(), [](auto e) {return e->val == "op"; });
					auto op = (*op_it)->op;
					childrenNodes.erase(op_it);
					t = new ASTNode{ {}, op, "", childrenNodes};
				}
				return t;
			}},
			{"Operator",
			[](std::vector<std::string> params, std::vector<ASTNode*> childrenNodes) {
				auto t = new ASTNode{{}, params[0],"op",{}};
				return t;
			}},
			{"Leaf",
			[&](std::vector<std::string> params, std::vector<ASTNode*> childrenNodes) {
				auto t = new ASTNode{{}, params[0], params[1], childrenNodes};
				if (params[0] == "ID")
					symbol_table.insert(params[1]);
				return t;
			}}
		};

		SematicProcesser(SematicLoader& loader, AnalyzeTree& tree):  loader(loader), atree(tree), ast(){}

		std::list<std::string> getChildrenProduction(AnalyzeTreeNode* node) {
			std::list<std::string> p;
			for (const auto& c : node->children) {
				p.push_back(c->symbol);
			}
			return std::move(p);
		}

		void traverse() {
			ast.root = traverse(atree.root);
		}
		ASTNode* traverse(AnalyzeTreeNode* node) {
			const auto& sematic = loader.GetSematic();
			std::vector<ASTNode*> castn;
			auto prod = std::make_pair(node->symbol, getChildrenProduction(node));
			auto s_it = sematic.find(prod);
			if (s_it == sematic.end()) {
				return nullptr;
			}
			for (const auto& cn : node->children) {
				auto t = traverse(cn);
				if(t!=nullptr)
					castn.push_back(t);
			}

			auto para = s_it->second.second;
			if (s_it->second.first == "Leaf")
				para[1] = node->children.front()->token.content;
			return actions[s_it->second.first](para, castn);
		}
	public:
		static AST AnalyzeToAST(SematicLoader& loader, AnalyzeTree& tree, std::set<std::string>& sym_table) {
			SematicProcesser processer(loader, tree);
			processer.traverse();
			sym_table = processer.symbol_table;
			return processer.ast;
		}
	};
}