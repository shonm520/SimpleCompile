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
            checkExpression(t->GetChildByIndex(i));
        switch (t->GetNodeKind())
        {
        case Parser::VAR_K:
        {
            SymbolTable::Info info = _pSymbolTable->subroutineTableFind(t->GetLexeme());
            if (info == SymbolTable::None)
            {
                info = _pSymbolTable->classesTableFind(_strCurClassName, t->GetLexeme());
                if (info == SymbolTable::None)
                {
                    error5(_strCurClassName, t->GetRow(), t->GetLexeme());
                }
            }
        }
        break;
        case Parser::ARRAY_K:
        {
            SymbolTable::Info info = _pSymbolTable->subroutineTableFind(t->GetLexeme());
            if (info == SymbolTable::None)
            {
                info = _pSymbolTable->classesTableFind(_strCurClassName, t->GetLexeme());
                if (info == SymbolTable::None)
                {
                    error5(_strCurClassName, t->GetRow(), t->GetLexeme());
                }
            }
            if (info.type != "Array")
            {
                error6(_strCurClassName, t->GetRow(), t->GetLexeme());
            }
        }
        break;
        case Parser::CALL_EXPRESSION_K:
        case Parser::CALL_STATEMENT_K:
        {
            if (t->GetLexeme().find('.') == string::npos)     // call_statement -> ID ( expressions ) 
            {
                // 先检查函数有没有在当前类中声明
                string functionName = t->GetLexeme();
                if (_pSymbolTable->classesTableFind(_strCurClassName, functionName) == SymbolTable::None)
                {
                    error7(_strCurClassName, _strCurClassName, t->GetRow(), functionName);
                    break;
                }
                SymbolTable::Kind currentFunctionKind = _pSymbolTable->classesTableFind(_strCurClassName, _strCurFunctionName).kind;
                SymbolTable::Kind calledFunctionKind = _pSymbolTable->classesTableFind(_strCurClassName, functionName).kind;
                // 再检查当前子过程和被调用过程是否都是method
                if (currentFunctionKind == SymbolTable::FUNCTION && calledFunctionKind == SymbolTable::FUNCTION)
                {
                    error8(_strCurClassName, t->GetRow(), functionName);
                    break;
                }
                // 再检查函数的参数是否正确
                SymbolTable::Info info = _pSymbolTable->classesTableFind(_strCurClassName, functionName);
                checkArguments(t, info.args, functionName);
                t->GetChildByIndex(0)->SetNodeKind(Parser::METHOD_CALL_K);
            }
            else                                            // call_statement -> ID . ID ( expressions ) 
            {
                // 先检查caller
                string callerName = Parser::getCallerName(t->GetLexeme());
                string functionName = Parser::getFunctionName(t->GetLexeme());
                if (_pSymbolTable->classIndexFind(callerName) == true)    // 如果caller是类
                {
                    // 再检查function
                    SymbolTable::Info info = _pSymbolTable->classesTableFind(callerName, functionName);
                    if (info == SymbolTable::None)
                    {
                        error7(_strCurClassName, callerName, t->GetRow(), functionName);
                        break;
                    }
                    if (info.kind == SymbolTable::METHOD)
                    {
                        error9(_strCurClassName, callerName, t->GetRow(), functionName);
                        break;
                    }
                    // 再检查参数
                    checkArguments(t, info.args, functionName);
                    if (info.kind == SymbolTable::FUNCTION)
                        t->GetChildByIndex(0)->SetNodeKind(Parser::FUNCTION_CALL_K);
                    else if (info.kind == SymbolTable::CONSTRUCTOR)
                        t->GetChildByIndex(0)->SetNodeKind(Parser::CONSTRUCTOR_CALL_K);
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
                            error5(_strCurClassName, t->GetRow(), callerName);
                            break;
                        }
                    }
                    // 再检查function
                    SymbolTable::Info functionInfo = _pSymbolTable->classesTableFind(objInfo.type, functionName);
                    if (functionInfo == SymbolTable::None)
                    {
                        error7(_strCurClassName, callerName, t->GetRow(), functionName);
                        break;
                    }
                    if (functionInfo.kind != SymbolTable::METHOD)
                    {
                        error10(_strCurClassName, callerName, t->GetRow(), functionName);
                        break;
                    }
                    // 再检查参数
                    checkArguments(t, functionInfo.args, functionName);
                    t->GetChildByIndex(0)->SetNodeKind(Parser::METHOD_CALL_K);
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
    switch (t->GetNodeKind())
    {
    case Parser::CLASS_K:
        _strCurClassName = t->GetChildByIndex(0)->GetLexeme();
        break;
    case Parser::ASSIGN_K:
    {
        checkExpression(t->GetChildByIndex(0));
        checkExpression(t->GetChildByIndex(1));
    }
    case Parser::IF_STATEMENT_K:
    case Parser::WHILE_STATEMENT_K:
    {
        checkExpression(t->GetChildByIndex(0));
        break;
    }
    case Parser::RETURN_STATEMENT_K:
    {
        checkExpression(t->GetChildByIndex(0));
        SymbolTable::Info info = _pSymbolTable->subroutineTableFind("this");
        if (t->GetChildByIndex(0) == nullptr && info.type != "void")
        {
            error11(_strCurClassName, info.type, t->GetRow());
            break;
        }
        else if (t->GetChildByIndex(0) != nullptr && info.type == "void")
        {
            error12(_strCurClassName, t->GetRow());
            break;
        }
        if (info.kind == SymbolTable::CONSTRUCTOR && t->GetChildByIndex(0)->GetLexeme() != "this")
        {
            error13(_strCurClassName, t->GetRow());
            break;
        }
        break;
    }
    case Parser::CALL_STATEMENT_K:
        checkExpression(t);
        break;
    }
}

void Analyzer::checkArguments(Parser::TreeNode *t, vector<string> parameter, string functionName)
{
    int argumentSize = 0;
    for (auto p = t->GetChildByIndex(0)->GetNextNode(); p != nullptr; p = p->GetNextNode())
    {
        checkExpression(p);
        argumentSize++;
    }
    if (argumentSize < parameter.size())
    {
        error14(_strCurClassName, functionName, t->GetRow());
        return;
    }
    else if (argumentSize > parameter.size())
    {
        error15(_strCurClassName, functionName, t->GetRow());
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
            buildClassesTable(t->GetChildByIndex(i));
            depth--;
        }
        t = t->GetNextNode();
    }
}

void Analyzer::checkStatements(Parser::TreeNode *t)
{
    while (t != nullptr)
    {
        _pSymbolTable->subroutineTableInsert(t);
        checkStatement(t);
        for (int i = 0; i < 5; i++)
            checkStatements(t->GetChildByIndex(i));
        t = t->GetNextNode();
    }
}
