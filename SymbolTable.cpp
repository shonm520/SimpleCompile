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
    if (t->GetNodeKind() == Parser::CLASS_K)
    {
        map<string, Info> temp;
        _vtClassesTable.push_back(temp);
        _strCurrentClass = t->GetChildByIndex(0)->GetLexeme();
        int index = _vtClassesTable.size() - 1;
        _mapClassIndex.insert({_strCurrentClass, index});
        _static_index = _field_index = 0;
    }
    else if (t->GetNodeKind() == Parser::CLASS_VAR_DEC_K)            // t = CLASS_VAR_DEC_K  
    {                                                           // t->GetChildByIndex(0) = static | field
        Info info;                                              // t->GetChildByIndex(1) = type          
        info.type = t->GetChildByIndex(1)->GetLexeme();                  // t->GetChildByIndex(2) = varName - varName - varName ...
        for (auto p = t->GetChildByIndex(2); p != nullptr; p = p->GetNextNode())
        {
            string name = p->GetLexeme();
            if (t->GetChildByIndex(0)->GetLexeme() == "field")
            {
                info.kind = FIELD;
                info.index = _field_index++;
            }
            else if (t->GetChildByIndex(0)->GetLexeme() == "static")
            {
                info.kind = STATIC;
                info.index = _static_index++;
            }

            if (_vtClassesTable.back().insert({ name, info }).second == false)   // ����ʧ��,���ű������Ѿ����ڵ�Ԫ��
            {
                error2(_strCurrentClass, p->GetRow(), info.type, name);
            }
        }
    }
    else if (t->GetNodeKind() == Parser::SUBROUTINE_DEC_K)           // t = SUBROUTINE_DEC_K
    {                                                           // t->GetChildByIndex(0) = function
        Info info;                                              // t->GetChildByIndex(1) = type
        if (t->GetChildByIndex(0)->GetLexeme() == "function")            // t->GetChildByIndex(2) = functionName
            info.kind = FUNCTION;                               // t->GetChildByIndex(3) = arg - arg - arg ...
        else if (t->GetChildByIndex(0)->GetLexeme() == "method")
            info.kind = METHOD;
        else if (t->GetChildByIndex(0)->GetLexeme() == "constructor")
            info.kind = CONSTRUCTOR;
        info.type = t->GetChildByIndex(1)->GetLexeme();
        for (auto p = t->GetChildByIndex(3); p != nullptr; p = p->GetNextNode())
        {
            string type = p->GetChildByIndex(0)->GetLexeme();
            info.args.push_back(type);
        }
        string name = Parser::getFunctionName(t->GetChildByIndex(2)->GetLexeme());
        if (_vtClassesTable.back().insert({ name, info }).second == false)
        {
            error3(_strCurrentClass, t->GetChildByIndex(0)->GetRow(), info.type, name);
        }
    }
}

void SymbolTable::subroutineTableInsert(Parser::TreeNode *t)
{
    if (t->GetNodeKind() == Parser::CLASS_K)
        _strCurrentClass = t->GetChildByIndex(0)->GetLexeme();
    else if (t->GetNodeKind() == Parser::SUBROUTINE_DEC_K)                               // t = SUBROUTINE_DEC_K
    {                                                                               // t->GetChildByIndex(0) = function
        initialSubroutineTable();                                                   // t->GetChildByIndex(1) = type
        string className = Parser::getCallerName(t->GetChildByIndex(2)->GetLexeme());        // t->GetChildByIndex(2) = functionName
        string functionName = Parser::getFunctionName(t->GetChildByIndex(2)->GetLexeme());   // t->GetChildByIndex(3) = arg - arg - arg ...
        _nCurrentClassNumber = _mapClassIndex.find(className)->second;
        Info info = _vtClassesTable[_nCurrentClassNumber].find(functionName)->second;
        _mapSubroutineTable["this"] = info;
        _var_index = _arg_index = 0;
    }
    else if (t->GetNodeKind() == Parser::PARAM_K)        // t = PARAM_K
    {                                               // t->GetChildByIndex(0) = type
        // �ȼ��type�Ƿ����                       // t->GetChildByIndex(1) = varName
        Info info;                                  
        info.kind = ARG;
        info.index = _arg_index++;
        info.type = t->GetChildByIndex(0)->GetLexeme();
        if (info.type != "int" && info.type != "char" && 
            info.type != "void" && info.type != "string" && info.type != "boolean")     // ������ǻ�������
        {
            if (classIndexFind(info.type) == false)     // Ҳ����������
            {
                error4(_strCurrentClass, t->GetChildByIndex(1)->GetRow(), info.type);
                return;
            }
        }
        // �ټ��varName�Ƿ����
        string varName = t->GetChildByIndex(1)->GetLexeme();
        if (_mapSubroutineTable.insert({ varName, info }).second == false)
        {
            error2(_strCurrentClass, t->GetChildByIndex(1)->GetRow(), info.type, varName);
            return;
        }
    }
    else if (t->GetNodeKind() == Parser::VAR_DEC_K)          // t = VAR_DEC_K
    {                                                   // t->GetChildByIndex(0) = type
        Info info;                                      // t->GetChildByIndex(1) = varName - varName - varName
        info.kind = VAR;
        info.type = t->GetChildByIndex(0)->GetLexeme();
        // �ȼ��type�Ƿ����
        if (info.type != "int" && info.type != "char" &&
            info.type != "void" && info.type != "string" && info.type != "boolean")
        {
            if (_mapClassIndex.find(info.type) == _mapClassIndex.end())
            {
                _errorNum++;
                error4(_strCurrentClass, t->GetChildByIndex(1)->GetRow(), info.type);
                return;
            }
        }
        // �ټ��varName�Ƿ����
        for (auto p = t->GetChildByIndex(1); p != nullptr; p = p->GetNextNode())
        {
            string varName = p->GetLexeme();
            info.index = _var_index++;
            if (_mapSubroutineTable.insert({ varName, info }).second == false)
            {
                error2(_strCurrentClass, p->GetRow(), info.type, varName);
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
    cout << "����\t\t���\t\t" << endl;
    for (auto iter = _mapClassIndex.cbegin(); iter != _mapClassIndex.cend(); ++iter)       
        cout << iter->first << "\t\t" << iter->second << endl;
    cout << endl;
    cout << "********************���ű�********************" << endl;
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
