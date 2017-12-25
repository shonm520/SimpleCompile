#include "Analyzer.h"
#include <iostream>

using namespace std;

Analyzer::Analyzer(Parser::TreeNode *t)
{
    _pSymbolTable = SymbolTable::getInstance();
    _pTree = t;
}

// �������ʽ��
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
					error22(t, "����������ǰ��ʹ��");
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
			string functionName = t->getLexeme();    // �ȼ�麯����û���ڵ�ǰ��������
			string strClassName = t->getClassName();
			string strCallerName;                    //����������Ҳ�������������
			size_t indexOfDot = functionName.find('.');
			if (indexOfDot != string::npos)  {       //����е�ŵ�������������(�����)�ͺ�����
				strCallerName = functionName.substr(0, indexOfDot);
				functionName = functionName.substr(indexOfDot + 1, functionName.size() - 1);
			}
			if (strCallerName == "")  {
				strCallerName = strClassName;
			}
			if (!_pSymbolTable->isClassType(strCallerName))  {    //����������,Ҳ�п������������
				SymbolTable::Info objInfo = _pSymbolTable->findInSubroutineTable(strCallerName);   
				if (objInfo == SymbolTable::None)  {              //���ں����ֲ�������
					objInfo = _pSymbolTable->findClassesTable(strClassName, strCallerName);   
					if (objInfo == SymbolTable::None)  {          //Ҳ������ľ�̬��Ա
						error5(strClassName, t->getRow(), strCallerName);
						break;
					}
				}
				strCallerName = objInfo.type;
			}
			SymbolTable::Info info = _pSymbolTable->findClassesTable(strCallerName, functionName);
			if (info == SymbolTable::None)  {
				error21(strClassName, strCallerName, t->getRow(), functionName);   //û�и÷�������
				break;
			}
			checkArguments(t, info.args, functionName);  //�������Ƿ�һ��

			if (info.kind == SymbolTable::CONSTRUCTOR)  {
				t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::CONSTRUCTOR_CALL_K);
			}
			else if (info.kind == SymbolTable::METHOD)  {
				t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::METHOD_CALL_K);
			}
			else if (info.kind == SymbolTable::FUNCTION)  {       //�Ǿ�̬����
				t->getChildByIndex(0)->setNodeKind(GramTreeNodeBase::FUNCTION_CALL_K);
			}
			break;
        }
        }
    }
}

/*
  ��鸳ֵ���, if���, while���, return���, �����������
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

void Analyzer::checkArguments(Parser::TreeNode *t, vector<string> parameter, string functionName)  //parameterΪ��������ʱ�Ĳ�������
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
