#include "SymbolTable.h"
#include <iostream>
#include <cassert>
#include "Error.h"

using namespace std;

SymbolTable::Info SymbolTable::None;

SymbolTable * SymbolTable::instance = nullptr;

SymbolTable * SymbolTable::getInstance()
{
    if (instance == nullptr)
        instance = new SymbolTable();
    return instance;
}

SymbolTable::SymbolTable()
{
    _static_index = 0;
    _field_index = 0;
    _arg_index = 0;
    _var_index = 0;
    _errorNum = 0;
}

bool SymbolTable::isBaseType(string type)
{
	if (type == "int" ||
		type == "char" ||
		type == "void" ||
		type == "string" ||
		type == "boolean")   {   // 如果不是基本类型  
		return true;
	}
	return false;
}


void SymbolTable::insertClassesTable(Parser::TreeNode *t)
{
    if (t->getNodeKind() == Parser::CLASS_K)
    {
        map<string, Info> temp;
        _vtClassesTable.push_back(temp);
        _strCurrentClass = t->getName();
        int index = _vtClassesTable.size() - 1;
        _mapClassIndex.insert({_strCurrentClass, index});
        _static_index = _field_index = 0;
		bulidClassMap(t->getClassName());
    }
    else if (t->getNodeKind() == Parser::CLASS_VAR_DEC_K)            // t = CLASS_VAR_DEC_K  
    {                                                           // t->getChildByIndex(0) = static | field
        Info info;                                              // t->getChildByIndex(1) = type          
		info.strClassName = t->getClassName();
		info.nodeIndex = t->getNodeIndex();
        info.type = t->getChildByIndex(1)->getLexeme();                  // t->getChildByIndex(2) = varName - varName - varName ...
        for (auto p = t->getChildByIndex(2); p != nullptr; p = p->getNextNode())
        {
            string name = p->getLexeme();
            if (t->getChildByIndex(0)->getLexeme() == "field")
            {
                info.kind = FIELD;
                info.index = _field_index++;
            }
            else if (t->getChildByIndex(0)->getLexeme() == "static")
            {
                info.kind = STATIC;
                info.index = _static_index++;
            }

			if (! insertClassMapInfo(t->getClassName(), name, info))
            {
                error2(_strCurrentClass, p->getRow(), info.type, name);
            }
        }
    }
    else if (t->getNodeKind() == Parser::SUBROUTINE_DEC_K)                       //在这里插入类函数声明的信息,函数的类型,返回值,名字等信息,不包括函数的局部变量    
    {                                                           
        Info info;                                             
		string signName = t->getSignName();
		if (signName == "function")            
            info.kind = FUNCTION;                            
		else if (signName == "method")
            info.kind = METHOD;
		else if (signName == "constructor")
            info.kind = CONSTRUCTOR;
		info.type = ((SubroutineDecNode*)t)->getRetType();
		for (auto p = ((SubroutineDecNode*)t)->getFirstParam(); p != nullptr; p = p->getNextNode())
        {
			string type = ((VarDecNode*)p)->getVarDecType()->getLexeme();
            info.args.push_back(type);
        }
        string name = Parser::getFunctionName(t->getName());
		if (! insertClassMapInfo(t->getClassName(), name, info))
        {
			error3(_strCurrentClass, t->getChildByIndex(SubroutineDecNode::SubroutineFiled::Sign)->getRow(), info.type, name);
        }
    }
}

void SymbolTable::insertWholeTreeTable(Parser::TreeNode *t)
{
	if (!t)  {
		return;
	}
	string strClassName;
	if (t)  {
		strClassName = t->getClassName();
	}
	if (t->getNodeKind() == Parser::CLASS_K)  {
		_strCurrentClass = t->getName();
		enterMapInfoBlock(t->getClassName(), t->getNodeIndex(), t->getClassName() + "_ClassArea_" + t->getName());
	}
    else if (t->getNodeKind() == GramTreeNodeBase::SUBROUTINE_DEC_K)                             
    {    
		enterMapInfoBlock(t->getClassName(), t->getNodeIndex(), t->getClassName() + "_Routine_" + t->getName());
        string className = Parser::getCallerName(t->getName());       
        string functionName = Parser::getFunctionName(t->getName());   
		Info info = _mapClassMapInfo[t->getClassName()]->find(functionName)->second;
		info.strClassName = t->getClassName();
		insertMapInfoInCurBlock(t->getClassName(), "this", info);
        _var_index = _arg_index = 0;
    }
    else if (t->getNodeKind() == Parser::PARAM_K)        
    {                                              
        Info info; 
		info.strClassName = strClassName;
        info.kind = ARG;
        info.index = _arg_index++;
		info.type = ((VarDecNode*)t)->getVarDecType()->getLexeme();
		if (!isBaseType(info.type))     // 如果不是基本类型
        {
            if (isClassType(info.type) == false)     // 也不是类类型
            {
				error4(_strCurrentClass, ((VarDecNode*)t)->getVarDecName()->getRow(), info.type);
                return;
            }
        }
		string varName = ((VarDecNode*)t)->getVarDecName()->getLexeme();
		if (! insertMapInfoInCurBlock(t->getClassName(), varName, info))
        {
			error2(_strCurrentClass, ((VarDecNode*)t)->getVarDecName()->getRow(), info.type, varName);
            return;
        }
    }
    else if (t->getNodeKind() == Parser::VAR_DEC_K)         
    {                                                   
        Info info;                                     
		info.strClassName = strClassName;
        info.kind = VAR;
		info.type = ((VarDecNode*)t)->getVarDecType()->getLexeme();
        if (!isBaseType(info.type))              // 先检查type是否合理
        {
			if (! isClassType(strClassName))
            {
                _errorNum++;
				error4(_strCurrentClass, ((VarDecNode*)t)->getVarDecName()->getRow(), info.type);
                return;
            }
        }
        
		for (auto p = ((VarDecNode*)t)->getVarDecName(); p != nullptr; p = p->getNextNode())  // 再检查varName是否合理
        {
            string varName = p->getLexeme();
            info.index = _var_index++;
			info.nodeIndex = p->getNodeIndex();
			if (!insertMapInfoInCurBlock(t->getClassName(), varName, info))
            {
                error2(_strCurrentClass, p->getRow(), info.type, varName);
            }
        }
    }
}

SymbolTable::Info SymbolTable::findInSubroutineTable(string name)
{
	stack<MapInfo*> tempStackInfo = _stackMapBlockTable;

	int num = _stackMapBlockTable.size();
	while (num > 0)  {
		auto mapInfoTable = _stackMapBlockTable.top();
		_stackMapBlockTable.pop();

		auto iter = mapInfoTable->find(name);
		if (iter != mapInfoTable->end())  {
			_stackMapBlockTable = tempStackInfo;
			return iter->second;
		}
		num = _stackMapBlockTable.size();
	}
	_stackMapBlockTable = tempStackInfo;
	return None;
}

SymbolTable::Info SymbolTable::findClassesTable(string className, string functionName)
{
    assert(isClassType(className));
    /*int classTableNumber = _mapClassIndex.find(className)->second;
    auto iter = _vtClassesTable[classTableNumber].find(functionName);
    if (iter == _vtClassesTable[classTableNumber].end())
        return None;
    else
        return iter->second;*/

	if (_mapClassMapInfo[className]->size() >= 0)  {
		auto iter = _mapClassMapInfo[className]->find(functionName);
		if (iter != _mapClassMapInfo[className]->end())  {
			return iter->second;
		}
		else  {
			return None;
		}
	}
	return None;
}

void SymbolTable::initialSubroutineTable()
{
    _mapSubroutineTable.clear();
}

void SymbolTable::printClassesTable()
{
    cout << "class index: " << endl;
    cout << "类名\t\t编号\t\t" << endl;
    for (auto iter = _mapClassIndex.cbegin(); iter != _mapClassIndex.cend(); ++iter)       
        cout << iter->first << "\t\t" << iter->second << endl;
    cout << endl;
    cout << "********************符号表********************" << endl;
    for (int i = 0; i < _vtClassesTable.size(); i++)
    {
        cout << "class table: " << i << endl;
        cout << "name\ttype\tkind\tvars" << endl;
        for (auto iter = _vtClassesTable[i].cbegin(); iter != _vtClassesTable[i].cend(); ++iter)
        {
            cout << iter->first << "\t" << iter->second.type << "\t" << iter->second.kind << "\t" << iter->second.index;
            for (int k = 0; k < iter->second.args.size(); ++k)
                cout << iter->second.args[k] << "\t";
            cout << endl;
        }
        cout << endl << endl;
    }

}

bool SymbolTable::isClassType(string className)
{
	return _mapClassMapInfo.find(className) != _mapClassMapInfo.end();
}

int SymbolTable::getFieldNumber(string className)
{
    assert(isClassType(className) == true);
//     int classNum = _mapClassIndex.find(className)->second;
//     int nField = 0;
//     for (auto iter = _vtClassesTable[classNum].cbegin(); iter != _vtClassesTable[classNum].cend(); ++iter)
//         if (iter->second.kind == FIELD)
//             nField++;
//     return nField;

	int nField = 0;
	auto mio = _mapClassMapInfo.find(className);
	if (mio != _mapClassMapInfo.end())  {
		for (auto p = mio->second->begin(); p != mio->second->end(); p++)  {
			if (p->second.kind == FIELD)  {
				nField++;
			}
		}
	}
	return nField;
}

SymbolTable::MapInfo* SymbolTable::getClassCurBlockMapInfoTable(string className)
{
	if (_stackMapBlockTable.size() == 0)  {
		return nullptr;
	}
	return _stackMapBlockTable.top();
}

void SymbolTable::bulidClassMap(string name)
{
	MapInfo* info = new MapInfo();
	_mapClassMapInfo[name] = info;
}

bool SymbolTable::insertClassMapInfo(string className, string name, Info info)
{
	return _mapClassMapInfo[className]->insert({ name, info }).second;
}

void SymbolTable::enterMapInfoBlock(string className, int index, string type)
{

	//printf("enter Block, Index:%d, type:%s\n", index, type.c_str());
	auto curMapInfo = getClassCurBlockMapInfoTable(className);
	if (curMapInfo && curMapInfo->index == index)  {
		return;
	}
	else  {
		MapInfo* info = new MapInfo();
		info->index = index;
		info->strAreaType = type;
		_stackMapBlockTable.push(info);
	}
}

bool SymbolTable::insertMapInfoInCurBlock(string className, string name, Info info)
{
	auto curMapInfo = getClassCurBlockMapInfoTable("");
	return curMapInfo->insert({ name, info }).second;
}


void SymbolTable::quitMapInfoBlock(int index, string type)
{
	auto p = _stackMapBlockTable.top();
	if (index != p->index)  {
		int b = 0;
		b = 1;
	}
	if (_stackMapBlockTable.size() == 6)  {
		int a = 0;
		a = 10;
	}
	_stackMapBlockTable.pop();
	delete p;
	
	//printf("quit  Block, Index:%d, type:%s\n", index, type.c_str());
}