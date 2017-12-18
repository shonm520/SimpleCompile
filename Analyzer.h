#ifndef _ANALYZER_H
#define _ANALYZER_H

#include "Parser.h"
#include "SymbolTable.h"
#include <vector>
#include "Error.h"

class Analyzer
{
private:
    Parser::TreeNode *_pTree;
    SymbolTable *_pSymbolTable;
    //string _strCurClassName;        // 遍历树的时候, 保存当前类的名称
    string _strCurFunctionName;     // 遍历树的时候, 保存当前函数的名称
    void buildClassesTable(Parser::TreeNode *t);
    void buildStatements(Parser::TreeNode *t);
    void checkStatement(Parser::TreeNode *t);
    void checkExpression(Parser::TreeNode *t);
    void checkArguments(Parser::TreeNode *t, vector<string> parameter, string functionName);
    void checkMain();
public:
    Analyzer(Parser::TreeNode *t);
    void check();
};

#endif
