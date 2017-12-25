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
		string strClassName;
        Kind kind;      // kind : static, field, var, argument 
        int index;
		int nodeIndex;  //��ǰ�ڵ��������,�����жϱ����Ƿ�������֮ǰʹ��
        vector<string> args;
        Info()
        {
            type = "0";
            kind = NONE;
            index = 0;
        }
        friend bool operator==(Info info1, Info info2)
        {
            if (info1.type == info2.type && 
				info1.kind == info2.kind &&
				info1.args == info2.args && 
				info1.strClassName == info2.strClassName)
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
    string _strCurrentClass;        // �����﷨����ʱ��, ���浱ǰ������
    void initialSubroutineTable();          // ���ٺ������ű�
    SymbolTable();
    static SymbolTable * instance;      // ָ����ű�ʵ������

	class Map : public map<string, Info>  {
	public:
		unsigned int index;
		string strAreaType;
	};

	typedef Map MapInfo;
	map<string, MapInfo*> _mapClassMapInfo;

	stack<MapInfo*> _stackMapBlockTable;

	bool isBaseType(string type);
public:
    static SymbolTable * getInstance();     // ���ط��ű�ʵ������
    void insertClassesTable(Parser::TreeNode *t);       // ����ű�Ĳ������
    void insertWholeTreeTable(Parser::TreeNode *t);    // �������ű�Ĳ������
    
    Info findInSubroutineTable(string name);  // �������ű�Ĳ��Ҳ���
    Info findClassesTable(string className, string functionName);   // ����ű�Ĳ��Ҳ���
    bool isClassType(string className);  // �ж�className�ǲ��ǺϷ�������
    
    int getFieldNumber(string className);
    void printClassesTable();       // ���Գ���, ��ӡ����ű�
	void bulidClassMap(string name);
	void enterMapInfoBlock(string className, int index, string type);
	MapInfo* getClassCurBlockMapInfoTable(string className);
	bool insertMapInfoInCurBlock(string className, string name, Info info);
	bool insertClassMapInfo(string className, string name, Info info);
	void quitMapInfoBlock(int index, string type);
};

#endif
