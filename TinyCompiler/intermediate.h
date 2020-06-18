#pragma once
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <utility>
#include <iomanip>
#include "SematicProcesser.h"
using namespace std;

struct Quad
{
	string op;
	string addr1;
	string addr2;
	string addr3;

	Quad(string a, string b="_", string c="_", string d="_")
	{
		op = a;
		addr1 = b;
		addr2 = c;
		addr3 = d;
	}
};

bool isOP(string op)
{
	if (op == "+" || op == "-" || op == "*")
		return true;
	else
		return false;
}
class genIR
{
private:
	hscp::AST ast;
	vector<Quad>quads;
	int tcount = 1;
	int lcount = 1;
public:
	genIR(hscp::AST a)
	{
		ast = a;
		traverse();
	}
	void traverse()
	{
		parseTree(ast.root);
	}
	//read write转换成四元式
	void parseWR(hscp::ASTNode* node)
	{
		if (node->op == "READ") {
			Quad temp("rd", node->children[0]->val);
			quads.push_back(temp);
		}
		else
		{
			Quad temp("wri", node->children[0]->val);
			quads.push_back(temp);
		}
	}

	void parseIF(hscp::ASTNode* node)
	{
		hscp::ASTNode* comp = node->children[0];

		//比较部分
		if (comp->op == ">")
		{
			Quad temp("gt", comp->children[0]->val, comp->children[1]->val, "t" + to_string(tcount++));
			quads.push_back(temp);
		}
		else if (comp->op == "==")
		{
			Quad temp("eq", comp->children[0]->val, comp->children[1]->val, "t" + to_string(tcount++));
			quads.push_back(temp);
		}
		else if (comp->op == "<")
		{
			Quad temp("lt", comp->children[0]->val, comp->children[1]->val, "t" + to_string(tcount++));
			quads.push_back(temp);
		}

		int lc = lcount;
		Quad f("if_f", "t" + to_string(tcount-1), "L"+to_string(lcount++) );
		quads.push_back(f);

		//true
		Quad t("lab", "L"+to_string(lcount++));
		quads.push_back(t);
		parseTree(node->children[0]);
		
		//false
		Quad fl("lab", "L" + to_string(lc));
		quads.push_back(fl);
		parseTree(node->children[1]);
	}
	void parseASN(hscp::ASTNode* node)
	{
		if (isOP(node->children[1]->op))
		{
			hscp::ASTNode* opNode = node->children[1];
			int tc = tcount;
			if (opNode->op == "+") {
				Quad temp("add", opNode->children[0]->val, opNode->children[1]->val, "t" + to_string(tcount++));
				quads.push_back(temp);
			}	
			else if (opNode->op == "-") {
				Quad temp("min", opNode->children[0]->val, opNode->children[1]->val, "t" + to_string(tcount++));
				quads.push_back(temp);
			}
				
			else if (opNode->op == "*") {
				Quad temp("mul", opNode->children[0]->val, opNode->children[1]->val, "t" + to_string(tcount++));
				quads.push_back(temp);
			}
			Quad t("asn", "t" + to_string(tc),node->children[0]->val);
			quads.push_back(t);
		}
		else
		{
			Quad temp("asn",node->children[0]->val,node->children[1]->val);
			quads.push_back(temp);
		}
	}

	void parseTree(hscp::ASTNode* node)
	{
		if (node == NULL)
		{
			Quad temp("halt");
			quads.push_back(temp);
			return;
		}
			
		if (node->op == "READ" || node->op == "WRITE")
			parseWR(node);
		else if (node->op == "IF")
			parseIF(node);
		else if (node->op == "ASSIGN")
			parseASN(node);
		else if (node->op == "sequence")
		{
			for (hscp::ASTNode* c : node->children)
				parseTree(c);
		}

		/*for(hscp::ASTNode*c:node->children)
			parseTree(c);*/
	}
	void PrintQuard()
	{
		cout << "四元式：" << endl;
		for (Quad q : quads)
		{
			cout << "(" << q.op << "," << q.addr1 << "," << q.addr2 << ", " << q.addr3 <<")"<< endl;
		}
	}
};
