#include "Analyzer.h"
#include <iostream>

using namespace std;

Analyzer::Analyzer(Parser::TreeNode *t)
{
    _pSymbolTable = SymbolTable::getInstance();
    _pTree = t;
}

// 遍历表达式树
void Analyzer::checkExpression(Parser::TreeNode *t)
{
    if (t != nullptr)
    {
        for (int i = 0; i < 5; i++)
            checkExpression(t->getChildByIndex(i));
        switch (t->getNodeKind())
        {
        case GramTreeNodeBase::VAR_K:
        {
            SymbolTable::Info info = _pSymbolTable->subroutineTableFind(t->getLexeme());
            if (info == SymbolTable::None)
            {
                info = _pSymbolTable->classesTableFind(_strCurClassName, t->getLexeme());
                if (info == SymbolTable::None)
                {
                    error5(_strCurClassName, t->getRow(), t->getLexeme());
                }
            }
        }
        break;
        case GramTreeNodeBase::ARRAY_K:
        {
            SymbolTable::Info info = _pSymbolTable->subroutineTableFind(t->getLexeme());
            if (info == SymbolTable::None)
            {
                info = _pSymbolTable->classesTableFind(_strCurClassName, t->getLexeme());
                if (info == SymbolTable::None)
                {
                    error5(_strCurClassName, t->getRow(), t->getLexeme());
                }
            }
            if (info.type != "Array")
            {
                error6(_strCurClassName, t->getRow(), t->getLexeme());
            }
        }
        break;
        case GramTreeNodeBase::CALL_EXPRESSION_K:
        case GramTreeNodeBase::CALL_STATEMENT_K:
        {
            if (t->getLexeme().find('.') == string::npos)     // call_statement -> ID ( expressions ) 
            {
                // 先检查函数有没有在当前类中声明
                string functionName = t->getLexeme();
                if (_pSymbolTable->classesTableFind(_strCurClassName, functionName) == SymbolTable::None)
                {
                    error7(_strCurClassName, _strCurClassName, t->getRow(), functionName);
                    break;
                }
                SymbolTable::Kind currentFunctionKind = _pSymbolTable->classesTableFind(_strCurClassName, _strCurFunctionName).kind;
                SymbolTable::Kind calledFunctionKind = _pSymbolTable->classesTableFind(_strCurClassName, functionName).kind;
                // 再检查当前子过程和被调用过程是否都是method
                if (currentFunctionKind == SymbolTable::FUNCTION && calledFunctionKind == SymbolTable::FUNCTION)
                {
                    error8(_strCurClassName, t->getRow(), functionName);
                    break;
                }
                // 再检查函数的参数是否正确
                SymbolTable::Info info = _pSymbolTable->classesTableFind(_strCurClassName, functionName);
                checkArguments(t, info.args, functionName);
				t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::METHOD_CALL_K);
            }
            else                                            // call_statement -> ID . ID ( expressions ) 
            {
                // 先检查caller
                string callerName = Parser::getCallerName(t->getLexeme());
                string functionName = Parser::getFunctionName(t->getLexeme());
                if (_pSymbolTable->classIndexFind(callerName) == true)    // 如果caller是类
                {
                    // 再检查function
                    SymbolTable::Info info = _pSymbolTable->classesTableFind(callerName, functionName);
                    if (info == SymbolTable::None)
                    {
                        error7(_strCurClassName, callerName, t->getRow(), functionName);
                        break;
                    }
                    if (info.kind == SymbolTable::METHOD)
                    {
                        error9(_strCurClassName, callerName, t->getRow(), functionName);
                        break;
                    }
                    // 再检查参数
                    checkArguments(t, info.args, functionName);
                    if (info.kind == SymbolTable::FUNCTION)
                        t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::FUNCTION_CALL_K);
                    else if (info.kind == SymbolTable::CONSTRUCTOR)
						t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::CONSTRUCTOR_CALL_K);
                }
                else                                                   // 如果调用者是对象
                {
                    // 再检查caller有没有被声明
                    SymbolTable::Info objInfo = _pSymbolTable->subroutineTableFind(callerName);
                    if (objInfo == SymbolTable::None)
                    {
                        objInfo = _pSymbolTable->classesTableFind(_strCurClassName, callerName);
                        if (objInfo == SymbolTable::None)
                        {
                            error5(_strCurClassName, t->getRow(), callerName);
                            break;
                        }
                    }
                    // 再检查function
                    SymbolTable::Info functionInfo = _pSymbolTable->classesTableFind(objInfo.type, functionName);
                    if (functionInfo == SymbolTable::None)
                    {
                        error7(_strCurClassName, callerName, t->getRow(), functionName);
                        break;
                    }
                    if (functionInfo.kind != SymbolTable::METHOD)
                    {
                        error10(_strCurClassName, callerName, t->getRow(), functionName);
                        break;
                    }
                    // 再检查参数
                    checkArguments(t, functionInfo.args, functionName);
					t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::METHOD_CALL_K);
//                    t->token.lexeme = objInfo.type + "." + functionName;
                }
            }
            break;
        }
        }
    }
}

/*
  检查赋值语句, if语句, while语句, return语句, 函数调用语句
*/
void Analyzer::checkStatement(Parser::TreeNode *t)
{
    switch (t->getNodeKind())
    {
    case GramTreeNodeBase::CLASS_K:
        _strCurClassName = t->getChildByIndex(0)->getLexeme();
        break;
    case GramTreeNodeBase::ASSIGN_K:
    {
        checkExpression(t->getChildByIndex(0));
        checkExpression(t->getChildByIndex(1));
    }
    case GramTreeNodeBase::IF_STATEMENT_K:
    case GramTreeNodeBase::WHILE_STATEMENT_K:
    {
        checkExpression(t->getChildByIndex(0));
        break;
    }
    case GramTreeNodeBase::RETURN_STATEMENT_K:
    {
        checkExpression(t->getChildByIndex(0));
        SymbolTable::Info info = _pSymbolTable->subroutineTableFind("this");
        if (t->getChildByIndex(0) == nullptr && info.type != "void")
        {
            error11(_strCurClassName, info.type, t->getRow());
            break;
        }
        else if (t->getChildByIndex(0) != nullptr && info.type == "void")
        {
            error12(_strCurClassName, t->getRow());
            break;
        }
        if (info.kind == SymbolTable::CONSTRUCTOR && t->getChildByIndex(0)->getLexeme() != "this")
        {
            error13(_strCurClassName, t->getRow());
            break;
        }
        break;
    }
    case GramTreeNodeBase::CALL_STATEMENT_K:
        checkExpression(t);
        break;
    }
}

void Analyzer::checkArguments(Parser::TreeNode *t, vector<string> parameter, string functionName)
{
    int argumentSize = 0;
    for (auto p = t->getChildByIndex(0)->getNextNode(); p != nullptr; p = p->getNextNode())
    {
        checkExpression(p);
        argumentSize++;
    }
    if (argumentSize < parameter.size())
    {
        error14(_strCurClassName, functionName, t->getRow());
        return;
    }
    else if (argumentSize > parameter.size())
    {
        error15(_strCurClassName, functionName, t->getRow());
        return;
    }
}

void Analyzer::check()
{
    buildClassesTable(_pTree);
//    symbolTable->printClassesTable();
    checkMain();
    checkStatements(_pTree);
}

void Analyzer::checkMain()
{
    if (_pSymbolTable->classIndexFind("Main") == false)
    {
        error16();
        return;
    }
    auto info = _pSymbolTable->classesTableFind("Main", "main");
    if (info == SymbolTable::None)
    {
        error17();
        return;
    }
    if (info.kind != SymbolTable::FUNCTION)
    {
        error18();
        return;
    }
    if (info.type != "void")
    {
        error19();
        return;
    }
    if (info.args.size() > 0)
    {
        error20();
        return;
    }
}

void Analyzer::buildClassesTable(Parser::TreeNode *t)
{
    static int depth = 0;
    if (depth > 2)
        return;
    while (t != nullptr)
    {
        _pSymbolTable->classesTableInsert(t);
        for (int i = 0; i < 5; i++)
        {
            depth++;
            buildClassesTable(t->getChildByIndex(i));
            depth--;
        }
        t = t->getNextNode();
    }
}

void Analyzer::checkStatements(Parser::TreeNode *t)
{
    while (t != nullptr)
    {
        _pSymbolTable->subroutineTableInsert(t);
        checkStatement(t);
        for (int i = 0; i < 5; i++)
            checkStatements(t->getChildByIndex(i));
        t = t->getNextNode();
    }
}
