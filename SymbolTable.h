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
        Kind kind;      // kind : static, field, var, argument 
        int index;
        vector<string> args;
        Info()
        {
            type = "0";
            kind = NONE;
            index = 0;
        }
        friend bool operator==(Info info1, Info info2)
        {
            if (info1.type == info2.type && info1.kind == info2.kind && info1.args == info2.args)
                return true;
            else
                return false;
        }
    };
    static Info None;
private:
    int _static_index;
    int _field_index;
    int _arg_index;
    int _var_index;
    int _errorNum;
    map<string, int> _mapClassIndex;                // ����������������
    vector<map<string, Info>> _vtClassesTable;     // ����ű�����, ��һֱ�����Ų��ᱻ����
    map<string, Info> _mapSubroutineTable;          // �������ű�
    int _nCurrentClassNumber;     // �����﷨����ʱ��, ���浱ǰ����ű���������
    string _strCurrentClass;        // �����﷨����ʱ��, ���浱ǰ������
    void initialSubroutineTable();          // ���ٺ������ű�
    SymbolTable();
    static SymbolTable * instance;      // ָ����ű�ʵ������

	bool isBaseType(string type);
public:
    static SymbolTable * getInstance();     // ���ط��ű�ʵ������
    void insertClassesTable(Parser::TreeNode *t);       // ����ű�Ĳ������
    void insertSubroutineTable(Parser::TreeNode *t);    // �������ű�Ĳ������
    
    Info findInSubroutineTable(string name);  // �������ű�Ĳ��Ҳ���
    Info findClassesTable(string className, string functionName);   // ����ű�Ĳ��Ҳ���
    bool isClassType(string className);  // �ж�className�ǲ��ǺϷ�������
    
    int getFieldNumber(string className);
    void printClassesTable();       // ���Գ���, ��ӡ����ű�
};

#endif
