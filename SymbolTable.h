#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include "Parser.h"
#include <map>
#include <vector>

class SymbolTable
{
public:
    enum Kind
    {
        STATIC, FIELD, ARG, VAR, FUNCTION, METHOD, CONSTRUCTOR, NONE
    };
    class Info
    {
    public:
        string type;    // int, float, char, string
		string strClassName;
        Kind kind;      // kind : static, field, var, argument 
        int index;
		int nodeIndex;  //当前节点的总索引,用于判断变量是否在声明之前使用
        vector<string> args;
        Info()
        {
            type = "0";
            kind = NONE;
            index = 0;
        }
        friend bool operator==(Info info1, Info info2)
        {
            if (info1.type == info2.type && 
				info1.kind == info2.kind &&
				info1.args == info2.args && 
				info1.strClassName == info2.strClassName)
                return true;
            else
                return false;
        }
    };
    static Info None;
private:
    int _errorNum;

    map<string, Info> _mapSubroutineTable;          // 函数符号表
    void initialSubroutineTable();          // 销毁函数符号表
    SymbolTable();
    static SymbolTable * instance;      // 指向符号表单实例对象

	class Map : public map<string, Info>  {
	public:
		unsigned int index;
		string strAreaType;
	};

	typedef Map MapInfo;
	map<string, MapInfo*> _mapClassMapInfo;

	stack<MapInfo*> _stackMapBlockTable;

	bool isBaseType(string type);
public:
    static SymbolTable * getInstance();     // 返回符号表单实例对象
    void insertClassesTable(Parser::TreeNode *t);       // 类符号表的插入操作
    void insertWholeTreeTable(Parser::TreeNode *t);    // 函数符号表的插入操作
    
    Info findInSubroutineTable(string name);  // 函数符号表的查找操作
    Info findClassesTable(string className, string functionName);   // 类符号表的查找操作
    bool isClassType(string className);  // 判断className是不是合法的类名
    
    int getFieldNumber(string className);
    void printClassesTable();       // 测试程序, 打印类符号表
	void bulidClassMap(string name);
	void enterMapInfoBlock(string className, int index, string type);
	MapInfo* getClassCurBlockMapInfoTable(string className);
	bool insertMapInfoInCurBlock(string className, string name, Info info);
	bool insertClassMapInfo(string className, string name, Info info);
	void quitMapInfoBlock(int index, string type);
};

#endif
