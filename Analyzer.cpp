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
			else  {
				if (info.nodeIndex > t->getNodeIndex())  {
					error22(t, "变量在声明前被使用");
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
			string functionName = t->getLexeme();    // 先检查函数有没有在当前类中声明
			string strClassName = t->getClassName();
			string strCallerName;                    //可能是类名也可能是类变量名
			size_t indexOfDot = functionName.find('.');
			if (indexOfDot != string::npos)  {       //如果有点号调用则分离出类名(类变量)和函数名
				strCallerName = functionName.substr(0, indexOfDot);
				functionName = functionName.substr(indexOfDot + 1, functionName.size() - 1);
			}
			if (strCallerName == "")  {
				strCallerName = strClassName;
			}
			if (!_pSymbolTable->isClassType(strCallerName))  {    //类名不存在,也有可能是类变量名
				SymbolTable::Info objInfo = _pSymbolTable->findInSubroutineTable(strCallerName);   
				if (objInfo == SymbolTable::None)  {              //不在函数局部变量中
					objInfo = _pSymbolTable->findClassesTable(strClassName, strCallerName);   
					if (objInfo == SymbolTable::None)  {          //也不是类的静态成员
						error5(strClassName, t->getRow(), strCallerName);
						break;
					}
				}
				strCallerName = objInfo.type;
			}
			SymbolTable::Info info = _pSymbolTable->findClassesTable(strCallerName, functionName);
			if (info == SymbolTable::None)  {
				error21(strClassName, strCallerName, t->getRow(), functionName);   //没有该方法或函数
				break;
			}
			checkArguments(t, info.args, functionName);  //检查参数是否一致

			if (info.kind == SymbolTable::CONSTRUCTOR)  {
				t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::CONSTRUCTOR_CALL_K);
			}
			else if (info.kind == SymbolTable::METHOD)  {
				t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::METHOD_CALL_K);
			}
			else if (info.kind == SymbolTable::FUNCTION)  {       //是静态函数
				t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::FUNCTION_CALL_K);
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

void Analyzer::checkArguments(Parser::TreeNode *t, vector<string> parameter, string functionName)  //parameter为函数声明时的参数个数
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
		if (t->isInCompoundBody(t))  {
			_pSymbolTable->enterMapInfoBlock(t->getClassName(), t->getNodeIndex(), t->getClassName() + "_Compound_Stmt");
		}
        _pSymbolTable->insertWholeTreeTable(t);
        checkStatement(t);
        for (int i = 0; i < 5; i++)
            buildStatements(t->getChildByIndex(i));

		if (t->getNodeKind() == GramTreeNodeBase::CLASS_K)  {
			string str = " class_area";
			_pSymbolTable->quitMapInfoBlock(t->getNodeIndex(), t->getClassName() + str);
		}
		else if (t->getNodeKind() == GramTreeNodeBase::SUBROUTINE_DEC_K)  {
			string str = " funtion_body";
			_pSymbolTable->quitMapInfoBlock(t->getNodeIndex(), t->getClassName() + str);
		}
		else if (t->isInCompoundBody(t))  {
			_pSymbolTable->quitMapInfoBlock(t->getNodeIndex(), t->getClassName() + " compound_body");
		}
        t = t->getNextNode();
    }
}
