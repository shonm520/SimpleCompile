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
    _nCurrentClassNumber = 0;
    _static_index = 0;
    _field_index = 0;
    _arg_index = 0;
    _var_index = 0;
    _errorNum = 0;
}


void SymbolTable::classesTableInsert(Parser::TreeNode *t)
{
    if (t->getNodeKind() == Parser::CLASS_K)
    {
        map<string, Info> temp;
        _vtClassesTable.push_back(temp);
        _strCurrentClass = t->getChildByIndex(0)->getLexeme();
        int index = _vtClassesTable.size() - 1;
        _mapClassIndex.insert({_strCurrentClass, index});
        _static_index = _field_index = 0;
    }
    else if (t->getNodeKind() == Parser::CLASS_VAR_DEC_K)            // t = CLASS_VAR_DEC_K  
    {                                                           // t->getChildByIndex(0) = static | field
        Info info;                                              // t->getChildByIndex(1) = type          
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

            if (_vtClassesTable.back().insert({ name, info }).second == false)   // 插入失败,符号表中有已经存在的元素
            {
                error2(_strCurrentClass, p->getRow(), info.type, name);
            }
        }
    }
    else if (t->getNodeKind() == Parser::SUBROUTINE_DEC_K)           // t = SUBROUTINE_DEC_K
    {                                                           // t->getChildByIndex(0) = function
        Info info;                                              // t->getChildByIndex(1) = type
        if (t->getChildByIndex(0)->getLexeme() == "function")            // t->getChildByIndex(2) = functionName
            info.kind = FUNCTION;                               // t->getChildByIndex(3) = arg - arg - arg ...
        else if (t->getChildByIndex(0)->getLexeme() == "method")
            info.kind = METHOD;
        else if (t->getChildByIndex(0)->getLexeme() == "constructor")
            info.kind = CONSTRUCTOR;
        info.type = t->getChildByIndex(1)->getLexeme();
        for (auto p = t->getChildByIndex(3); p != nullptr; p = p->getNextNode())
        {
            string type = p->getChildByIndex(0)->getLexeme();
            info.args.push_back(type);
        }
        string name = Parser::getFunctionName(t->getChildByIndex(2)->getLexeme());
        if (_vtClassesTable.back().insert({ name, info }).second == false)
        {
            error3(_strCurrentClass, t->getChildByIndex(0)->getRow(), info.type, name);
        }
    }
}

void SymbolTable::subroutineTableInsert(Parser::TreeNode *t)
{
    if (t->getNodeKind() == Parser::CLASS_K)
        _strCurrentClass = t->getChildByIndex(0)->getLexeme();
    else if (t->getNodeKind() == GramTreeNodeBase::SUBROUTINE_DEC_K)                               // t = SUBROUTINE_DEC_K
    {                                                                               // t->getChildByIndex(0) = function
        initialSubroutineTable();                                                   // t->getChildByIndex(1) = type
        string className = Parser::getCallerName(t->getChildByIndex(2)->getLexeme());        // t->getChildByIndex(2) = functionName
        string functionName = Parser::getFunctionName(t->getChildByIndex(2)->getLexeme());   // t->getChildByIndex(3) = arg - arg - arg ...
        _nCurrentClassNumber = _mapClassIndex.find(className)->second;
        Info info = _vtClassesTable[_nCurrentClassNumber].find(functionName)->second;
        _mapSubroutineTable["this"] = info;
        _var_index = _arg_index = 0;
    }
    else if (t->getNodeKind() == Parser::PARAM_K)        // t = PARAM_K
    {                                               // t->getChildByIndex(0) = type
        // 先检查type是否合理                       // t->getChildByIndex(1) = varName
        Info info;                                  
        info.kind = ARG;
        info.index = _arg_index++;
        info.type = t->getChildByIndex(0)->getLexeme();
        if (info.type != "int" && info.type != "char" && 
            info.type != "void" && info.type != "string" && info.type != "boolean")     // 如果不是基本类型
        {
            if (classIndexFind(info.type) == false)     // 也不是类类型
            {
                error4(_strCurrentClass, t->getChildByIndex(1)->getRow(), info.type);
                return;
            }
        }
        // 再检查varName是否合理
        string varName = t->getChildByIndex(1)->getLexeme();
        if (_mapSubroutineTable.insert({ varName, info }).second == false)
        {
            error2(_strCurrentClass, t->getChildByIndex(1)->getRow(), info.type, varName);
            return;
        }
    }
    else if (t->getNodeKind() == Parser::VAR_DEC_K)          // t = VAR_DEC_K
    {                                                   // t->getChildByIndex(0) = type
        Info info;                                      // t->getChildByIndex(1) = varName - varName - varName
        info.kind = VAR;
        info.type = t->getChildByIndex(0)->getLexeme();
        // 先检查type是否合理
        if (info.type != "int" && info.type != "char" &&
            info.type != "void" && info.type != "string" && info.type != "boolean")
        {
            if (_mapClassIndex.find(info.type) == _mapClassIndex.end())
            {
                _errorNum++;
                error4(_strCurrentClass, t->getChildByIndex(1)->getRow(), info.type);
                return;
            }
        }
        // 再检查varName是否合理
        for (auto p = t->getChildByIndex(1); p != nullptr; p = p->getNextNode())
        {
            string varName = p->getLexeme();
            info.index = _var_index++;
            if (_mapSubroutineTable.insert({ varName, info }).second == false)
            {
                error2(_strCurrentClass, p->getRow(), info.type, varName);
            }
        }
    }
}

SymbolTable::Info SymbolTable::subroutineTableFind(string name)
{
    auto iter = _mapSubroutineTable.find(name);
    if (iter == _mapSubroutineTable.end())
        return None;
    else
        return iter->second;
}

SymbolTable::Info SymbolTable::classesTableFind(string className, string functionName)
{
    assert(classIndexFind(className) == true);
    int classTableNumber = _mapClassIndex.find(className)->second;
    auto iter = _vtClassesTable[classTableNumber].find(functionName);
    if (iter == _vtClassesTable[classTableNumber].end())
        return None;
    else
        return iter->second;
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

bool SymbolTable::classIndexFind(string className)
{
    if (_mapClassIndex.find(className) == _mapClassIndex.end())
        return false;
    else
        return true;
}

int SymbolTable::getFieldNumber(string className)
{
    assert(classIndexFind(className) == true);
    int classNum = _mapClassIndex.find(className)->second;
    int nField = 0;
    for (auto iter = _vtClassesTable[classNum].cbegin(); iter != _vtClassesTable[classNum].cend(); ++iter)
        if (iter->second.kind == FIELD)
            nField++;
    return nField;
}
