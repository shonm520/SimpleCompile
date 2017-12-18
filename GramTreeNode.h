#pragma once
#include <string>
#include <map>
using std::string;

#include "Scanner.h"

class GramTreeNodeBase
{
public:
	static const int Child_Num_Const = 5;
	enum NodeKind {
		None, 
		CLASS_K,
		CLASS_VAR_DEC_K,
		SUBROUTINE_DEC_K,
		BASIC_TYPE_K,
		CLASS_TYPE_K,
		NULL_K,
		PARAM_K,
		VAR_DEC_K,
		ARRAY_K,
		VAR_K,
		IF_STATEMENT_K,
		WHILE_STATEMENT_K,
		CALL_EXPRESSION_K,
		RETURN_STATEMENT_K,
		CALL_STATEMENT_K,
		BOOL_EXPRESSION_K,
		FUNCTION_CALL_K,
		CONSTRUCTOR_CALL_K,
		COMPARE_K,
		OPERATION_K,
		BOOL_K,
		ASSIGN_K,
		SUBROUTINE_BODY_K,
		BOOL_CONST_K,
		NEGATIVE_K, 
		METHOD_CALL_K,
		INT_CONST_K,
		CHAR_CONST_K,
		STRING_CONST_K,
		KEY_WORD_CONST,
		THIS_K
	};

	static string Kind2Des(int nK)  {
		static map<int, string> mapStrDes;
		if (mapStrDes.size() == 0) {
			mapStrDes.insert({ CLASS_K, "类类型" });
			mapStrDes.insert(make_pair(CLASS_VAR_DEC_K, "类变量声明"));
			mapStrDes.insert(make_pair(SUBROUTINE_DEC_K, "函数定义"));
			mapStrDes.insert(make_pair(BASIC_TYPE_K, "基本类型"));
			mapStrDes.insert(make_pair(PARAM_K, "参数类型"));
			mapStrDes.insert(make_pair(VAR_DEC_K, "变量声明"));
			mapStrDes.insert(make_pair(ARRAY_K, "数组类型"));
			mapStrDes.insert(make_pair(VAR_K, "变量"));
			mapStrDes.insert(make_pair(SUBROUTINE_BODY_K, "函数体"));
			mapStrDes.insert(make_pair(RETURN_STATEMENT_K, "返回语句"));
			mapStrDes.insert(make_pair(CALL_STATEMENT_K, "调用语句"));
			mapStrDes.insert(make_pair(ASSIGN_K, "赋值语句"));
			mapStrDes.insert(make_pair(IF_STATEMENT_K, "if语句"));
			mapStrDes.insert(make_pair(WHILE_STATEMENT_K, "while语句"));
			mapStrDes.insert(make_pair(COMPARE_K, "比较体"));
			mapStrDes.insert(make_pair(OPERATION_K, "操作符"));
			mapStrDes.insert(make_pair(INT_CONST_K, "常整数"));
		}
		return mapStrDes[nK];
	}

public:
	GramTreeNodeBase(int nK = None)  {
		_pNext = nullptr;
		_nodeKind = (NodeKind)nK;
		_strNodeDes = Kind2Des(nK);
		_strClassName = "";
		_childIndex = -1;
		_siblings = 0;
		_pParent = nullptr;
		memset(&_child, 0, sizeof(GramTreeNodeBase*)* Child_Num_Const);
	}
	~GramTreeNodeBase(){}

protected:
	GramTreeNodeBase* _child[Child_Num_Const];
	GramTreeNodeBase* _pParent;
	GramTreeNodeBase* _pNext;
	NodeKind _nodeKind;
	string   _strNodeDes;
	string   _strClassName;
	short    _childIndex;     //子类的索引
	short    _siblings;       //同等节点的个数
	  
	Scanner::Token token_;

public:
	void setToken(Scanner::Token& t)  {
		token_ = t;
	}
	const string getLexeme()  {
		return token_.lexeme;
	}
	void setLexeme(string& str)  {
		token_.lexeme = str;
	}
	unsigned int getRow()  {
		return token_.row;
	}
	void setNodeKind(int kind)  {
		_nodeKind = (NodeKind)kind;
	}
	NodeKind getNodeKind()  {
		return _nodeKind;
	}
	void setNextNode(GramTreeNodeBase* node)  {
		if (node)  {
			node->_childIndex = _childIndex;   //这么理解,父亲有多个老婆,每个老婆有0到多个儿子,生母是同一个的孩子编号一样
		}
		_pNext = node;
	}
	short getSiblings()  {
		return _siblings;
	}
	GramTreeNodeBase* getNextNode()  {
		return _pNext;
	}
	GramTreeNodeBase* getParentNode()  {
		return _pParent;
	}

	void addChild(GramTreeNodeBase* pChild, int ind = -1);

	string getClassName();

	GramTreeNodeBase* getChildByIndex(int ind)  {
		if (ind >= 0 && ind < Child_Num_Const)  {
			return _child[ind];
		}
		return nullptr;
	}

	virtual string getName() { return ""; }
	virtual string getSignName() { return ""; }
	virtual GramTreeNodeBase* getChildByTag(string name) { return nullptr; }
	
};





class ClassTreeNode : public GramTreeNodeBase  {   //类类型节点,所有类型的最终parent指向该类
public:
	ClassTreeNode(int nK = None) : GramTreeNodeBase(nK)  {
		_nodeKind = CLASS_K;
	}
	virtual ~ClassTreeNode(){}

	enum ClassFiled  {
		Class_Name = 0,
		Class_VarDec,
		Class_SubroutineDec
	}; 

	virtual string getName() override;
	string getClassName()  {
		return getName();
	}

private:
	string _strClassName;
};


class SubroutineBodyNode;

class SubroutineDecNode : public GramTreeNodeBase  {     //整个函数的节点
public:
	SubroutineDecNode() : GramTreeNodeBase()  {
		_nodeKind = SUBROUTINE_DEC_K;
	}
	virtual ~SubroutineDecNode(){}

	enum  SubroutineFiled  {
		Sign = 0,
		Ret,
		Name,
		Params,
		Body
	};

	virtual string getName() override;
	virtual string getSignName() override;
	string getRetType();
	GramTreeNodeBase* getFirstParam();
	int getFuncLocalsNum();
	SubroutineBodyNode* getSubroutineBody();
};


class SubroutineBodyNode : public GramTreeNodeBase  {    //函数体节点
public:
	SubroutineBodyNode() : GramTreeNodeBase()  {
		_nodeKind = SUBROUTINE_BODY_K;
	}
	virtual ~SubroutineBodyNode(){}

	enum SubroutineBody  {
		VarDec = 0,
		Statement
	};

	int getFuncLocalsNum();
};


class AssignStatement : public GramTreeNodeBase  {    //赋值语句节点
public:
	AssignStatement() : GramTreeNodeBase()  {
		_nodeKind = ASSIGN_K;
	}
	virtual ~AssignStatement(){}
	enum AStatement  {
		AssignLetf = 0,
		AssignRight
	};

	virtual GramTreeNodeBase* getChildByTag(string name) override;
	GramTreeNodeBase* getAssginLeft();
	GramTreeNodeBase* getAssginRight();
};




class VarDecNode : public GramTreeNodeBase  {   //变量声明节点
public:
	VarDecNode() : GramTreeNodeBase()  {
		_nodeKind = VAR_DEC_K;
	}
	enum EVarDec  {
		VarDec_Type = 0,
		VarDec_Name
	};
	virtual ~VarDecNode(){}
	GramTreeNodeBase* getVarDecType();    //变量的声明
	GramTreeNodeBase* getVarDecName();    //变量的名字

};




class ParamNode : public VarDecNode  {    //形参节点
public:
	ParamNode() : VarDecNode()  {
		_nodeKind = PARAM_K;
	}
	virtual ~ParamNode(){}
};

