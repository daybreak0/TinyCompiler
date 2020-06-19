#pragma once

#include<iostream>
#include<vector>

using namespace std;
struct Quad{
    string op;
    string addr1;//目标地址
    string addr2;//源地址
    string addr3;//源地址
    int block_id;
    Quad(string a, string b="_", string c="_",string d="_")
    {
        op = a;
        addr1 = b;
        addr2 = c;
        addr3 = d;
    }
    //划分基本块
    void divide_basic_block(vector<Quad> qua);
    //判断String是否是常数
    bool judgeNum(string x)
    {
        ///not
        //传进来x，返回x是否是常数的判断
        for(int i=0;i<x.size();i++)
        {
            if(!(x[i]<='9'&&x[i]>='0'))
            {
                return false;
            }
        }
        return true;
    }
    //将string转换成int类
    int transStringToInt(string x)
    {
        ///not
        int res=0;
        for(int i=x.size()-1;i>=0;i--)
        {
            res+=(x[i]-'0');
            if(i!=(x.size()-1)) res*=10;
        }
        return res;
    }
    
    //给两个操作数，一个op，计算结果
    //前提：已判断过两个string都是数字
    int calculation(string x,string y,string op)
    {
        int xx=transStringToInt(x);
        int yy=transStringToInt(y);
        switch (op)
        {
        case "+":
            /* code */
            return xx+yy;
            break;
        case "-":
            /* code */
            return xx-yy;
            break;
        case "*":
            /* code */
            return xx*yy;
            break;
        default:
            break;
        }
    }
    //常数合并
    void ConstantMerge(vector<Quad> v);
    //常数传播
    void ConstantPropagation(vector<Quad> v);
    //代数简化
    void AlgebraicSimplification(vector<Quad> v);
};
Quad::divide_basic_block(vector<Quad> qua)
{
    
    /*
    param：四元式集合, vector
    return: None
    */
    int i=0;
    int num=1;

    // 跳转标识集合
    set<string> set_1;
    set_1.insert("L1");
    set_1.insert("L2");
    set_1.insert("L3");

    //判断入口语句集合
    set<string> set_2;
    set_2.insert("if");
    set_2.insert("else");
    set_2.insert("repeat");
    set_2.insert("util");
    set_2.insert("goto");

    //遍历集合找入口语句
    while(qua[i].op!="")        
    {

        //跳转语句情况下 且前一句不在入口语句集合下+1(避免重复加)
        if(set_1.count(qua[i].op))       
            if(i>0&&!set_2.count(qua[i-1].op))
                num++; 
        qua[i].block_id=num; 
        //如果操作符不在集合中，则基本块+1
        if(set_2.count(qua[i].op)) num++;  
        i++;
    }
    //print
    for(int i=0;i<qua.size();i++)
    {
        cout << qua[i].op << " " << qua[i].addr1 << " " << qua[i].addr2 << " " <<qua[i].addr3 << " |"<<qua[i].block_id;
        cout  << endl;

    }

}

Quad::ConstantMerge(vector<Quad> v)
{
    for(int i=0;i<v.size();i++)
    {
        //如果这条语句的两个地址码都是数字，代表可以简化
        if(judgeNum(v[i].addr2)&&judgeNum(v[i].addr3))
        {
            //计算出结果
            int res=calculation(v[i].addr2,v[i].addr3,v[i].op);
            //addr2赋值成res，addr3变为_
            v[i].addr2=to_string(res);
            v[i].addr3="_";
        }
    }

}
Quad::ConstantPropagation(vector<Quad> v)
{
    for(int i=0;i<v.size()-1;i++)
    {
        //如果前一个语句的目标地址=后一个的源地址
        //则用前一个的目标地址的源地址代替后一个的源地址

        //只在基本块内做优化
        if(v[i].block_id==v[i+1].block_id)
        {
            //地址相等
            if(v[i].addr1==v[i+1].addr2){
                //赋值
                v[i+1].addr2=v[i].addr2;
                //删除前一条语句
                v.erase(v[i]);
                i--;
            }
        }
    }
}

Quad::AlgebraicSimplification(vector<Quad> v)
{
    for(int i=0;i<v.size();i++)
    {
        if(v[i].op=="add")
        {
            //x=0+x
            if(v[i].addr2=="0")
            {
                
                if(v[i].addr3!="_")
                {
                    v[i].addr2=v[i].addr3;
                    v[i].addr3="_";
                }
            }
            //x=x+0
            else if(v[i].addr3=="0") v[i].addr3="_";               

        }
        if(v[i].op=="mul")
        {
            //x=1*x
            if(v[i].addr2=="1")
            {
                
                if(v[i].addr3!="_")
                {
                    v[i].addr2=v[i].addr3;
                    v[i].addr3="_";
                }
            }
            //x=x*1
            else if(v[i].addr3=="1") v[i].addr3="_";
        }
        if(v[i].op=="sub")
        {
            //x=x-0
            if(v[i].addr3=="0") v[i].addr3="_";
        }
        if(v[i].op=="and")
        {
            // b && false=false
            if(v[i].addr2=="false"||v[i].addr3=="false")
            {
                v[i].addr2="false";
                v[i].addr3="_";
            }
            // true && b = b
            if(v[i].addr2=="true")
            {
                v[i].addr2=v[i].addr3;
                v[i].addr2="_";
            }
            // b && true = b
            if(v[i].addr3=="true")
            {
                v[i].addr3=="_"
            }
        }
        if(v[i].op=="or")
        {
            // b || true = true
            if(v[i].addr2=="true"||v[i].addr3=="true")
            {
                v[i].addr2="true";
                v[i].addr3="_";
            }
            //false || b = false
            if(v[i].addr2=="false")
            {
                v[i].addr2=v[i].addr3;
                v[i].addr3="_";
            }
            //b || false = false
            else if(v[i].addr3=="false")
                v[i].addr3="_";

        }
    }
}