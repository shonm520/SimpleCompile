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

		string strClassName = t->getClassName();
        switch (t->getNodeKind())
        {
        case GramTreeNodeBase::VAR_K:
        {
            SymbolTable::Info info = _pSymbolTable->findInSubroutineTable(t->getLexeme());
            if (info == SymbolTable::None)
            {
				info = _pSymbolTable->findClassesTable(strClassName, t->getLexeme());
                if (info == SymbolTable::None)
                {
					error5(strClassName, t->getRow(), t->getLexeme());
                }
            }
        }
        break;
        case GramTreeNodeBase::ARRAY_K:
        {
            SymbolTable::Info info = _pSymbolTable->findInSubroutineTable(t->getLexeme());
            if (info == SymbolTable::None)
            {
				info = _pSymbolTable->findClassesTable(strClassName, t->getLexeme());
                if (info == SymbolTable::None)
                {
					error5(strClassName, t->getRow(), t->getLexeme());
                }
            }
            if (info.type != "Array")
            {
				error6(strClassName, t->getRow(), t->getLexeme());
            }
        }
        break;
        case GramTreeNodeBase::CALL_EXPRESSION_K:
        case GramTreeNodeBase::CALL_STATEMENT_K:
        {
            if (t->getLexeme().find('.') == string::npos)     // call_statement -> ID ( expressions ) 
            {
                string functionName = t->getLexeme();    // 先检查函数有没有在当前类中声明
				if (_pSymbolTable->findClassesTable(strClassName, functionName) == SymbolTable::None)
                {
					error7(strClassName, strClassName, t->getRow(), functionName);
                    break;
                }
				SymbolTable::Kind currentFunctionKind = _pSymbolTable->findClassesTable(strClassName, _strCurFunctionName).kind;
                SymbolTable::Kind calledFunctionKind = _pSymbolTable->findClassesTable(strClassName, functionName).kind;
                
                if (currentFunctionKind == SymbolTable::FUNCTION && calledFunctionKind == SymbolTable::FUNCTION)   // 再检查当前子过程和被调用过程是否都是method
                {
                    error8(strClassName, t->getRow(), functionName);
                    break;
                }
                SymbolTable::Info info = _pSymbolTable->findClassesTable(strClassName, functionName);         // 再检查函数的参数是否正确
                checkArguments(t, info.args, functionName);
				t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::METHOD_CALL_K);
            }
            else                                            // call_statement -> ID . ID ( expressions ) 
            {
                string callerName = Parser::getCallerName(t->getLexeme());      // 先检查caller
                string functionName = Parser::getFunctionName(t->getLexeme());
                if (_pSymbolTable->isClassType(callerName) == true)             // 如果caller是类
                {
                    SymbolTable::Info info = _pSymbolTable->findClassesTable(callerName, functionName);   // 再检查function
                    if (info == SymbolTable::None)
                    {
                        error7(strClassName, callerName, t->getRow(), functionName);
                        break;
                    }
                    if (info.kind == SymbolTable::METHOD)
                    {
                        error9(strClassName, callerName, t->getRow(), functionName);
                        break;
                    }
                    checkArguments(t, info.args, functionName);  // 再检查参数
                    if (info.kind == SymbolTable::FUNCTION)
                        t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::FUNCTION_CALL_K);
                    else if (info.kind == SymbolTable::CONSTRUCTOR)
						t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::CONSTRUCTOR_CALL_K);
                }
                else                                                   // 如果调用者是对象
                {
                    SymbolTable::Info objInfo = _pSymbolTable->findInSubroutineTable(callerName);  // 再检查caller有没有被声明
                    if (objInfo == SymbolTable::None)
                    {
                        objInfo = _pSymbolTable->findClassesTable(strClassName, callerName);
                        if (objInfo == SymbolTable::None)
                        {
                            error5(strClassName, t->getRow(), callerName);
                            break;
                        }
                    }
                    SymbolTable::Info functionInfo = _pSymbolTable->findClassesTable(objInfo.type, functionName);   // 再检查function
                    if (functionInfo == SymbolTable::None)
                    {
                        error7(strClassName, callerName, t->getRow(), functionName);
                        break;
                    }
                    if (functionInfo.kind != SymbolTable::METHOD)
                    {
                        error10(strClassName, callerName, t->getRow(), functionName);
                        break;
                    }
                    checkArguments(t, functionInfo.args, functionName);   // 再检查参数
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
	string strClassName = t->getClassName();
    switch (t->getNodeKind())
    {
    case GramTreeNodeBase::CLASS_K:
		strClassName = t->getName();
        break;
    case GramTreeNodeBase::ASSIGN_K:
    {
        checkExpression(((AssignStatement*)t)->getAssginLeft());
		checkExpression(((AssignStatement*)t)->getAssginRight());
		break;
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
        SymbolTable::Info info = _pSymbolTable->findInSubroutineTable("this");
        if (t->getChildByIndex(0) == nullptr && info.type != "void")
        {
			error11(strClassName, info.type, t->getRow());
            break;
        }
        else if (t->getChildByIndex(0) != nullptr && info.type == "void")
        {
			error12(strClassName, t->getRow());
            break;
        }
        if (info.kind == SymbolTable::CONSTRUCTOR && t->getChildByIndex(0)->getLexeme() != "this")
        {
			error13(strClassName, t->getRow());
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

	string strClassName = t->getClassName();
    if (argumentSize < parameter.size())
    {
		error14(strClassName, functionName, t->getRow());
        return;
    }
    else if (argumentSize > parameter.size())
    {
		error15(strClassName, functionName, t->getRow());
        return;
    }
}

void Analyzer::check()
{
    buildClassesTable(_pTree);
//    symbolTable->printClassesTable();
    checkMain();
    buildStatements(_pTree);
}

void Analyzer::checkMain()
{
    if (_pSymbolTable->isClassType("Main") == false)
    {
        error16();
        return;
    }
    auto info = _pSymbolTable->findClassesTable("Main", "main");
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
        _pSymbolTable->insertClassesTable(t);
        for (int i = 0; i < 5; i++)
        {
            depth++;
            buildClassesTable(t->getChildByIndex(i));
            depth--;
        }
        t = t->getNextNode();
    }
}

void Analyzer::buildStatements(Parser::TreeNode *t)
{
    while (t != nullptr)
    {
        _pSymbolTable->insertSubroutineTable(t);
        checkStatement(t);
        for (int i = 0; i < 5; i++)
            buildStatements(t->getChildByIndex(i));
        t = t->getNextNode();
    }
}
