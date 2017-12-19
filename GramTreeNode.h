#pragma once
#include <string>
#include <map>
#include <stack>
#include <cassert>
using std::string;
using std::stack;

#include "Scanner.h"

class SubroutineBodyNode;
class CompondStatement;

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
			mapStrDes.insert({ CLASS_K, "������" });
			mapStrDes.insert(make_pair(CLASS_VAR_DEC_K, "���������"));
			mapStrDes.insert(make_pair(SUBROUTINE_DEC_K, "��������"));
			mapStrDes.insert(make_pair(BASIC_TYPE_K, "��������"));
			mapStrDes.insert(make_pair(PARAM_K, "��������"));
			mapStrDes.insert(make_pair(VAR_DEC_K, "��������"));
			mapStrDes.insert(make_pair(ARRAY_K, "��������"));
			mapStrDes.insert(make_pair(VAR_K, "����"));
			mapStrDes.insert(make_pair(SUBROUTINE_BODY_K, "������"));
			mapStrDes.insert(make_pair(RETURN_STATEMENT_K, "�������"));
			mapStrDes.insert(make_pair(CALL_STATEMENT_K, "�������"));
			mapStrDes.insert(make_pair(ASSIGN_K, "��ֵ���"));
			mapStrDes.insert(make_pair(IF_STATEMENT_K, "if���"));
			mapStrDes.insert(make_pair(WHILE_STATEMENT_K, "while���"));
			mapStrDes.insert(make_pair(COMPARE_K, "�Ƚ���"));
			mapStrDes.insert(make_pair(OPERATION_K, "������"));
			mapStrDes.insert(make_pair(INT_CONST_K, "������"));
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
	short    _childIndex;     //���������
	short    _siblings;       //ͬ�Ƚڵ�ĸ���
	  
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
			node->_childIndex = _childIndex;   //��ô���,�����ж������,ÿ��������0���������,��ĸ��ͬһ���ĺ��ӱ��һ��
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
	void setParentNode(GramTreeNodeBase* node)  {
		_pParent = node;
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

	static stack<SubroutineBodyNode*> s_stackCurSubroutineZone ;          //��ǰ�ĺ���������
	static SubroutineBodyNode* getCurSubroutineBodyNode()  {
		if (s_stackCurSubroutineZone.size() == 0)  {
			return nullptr;
		}
		return s_stackCurSubroutineZone.top();
	}
	static void insertSubRoutineBodyNode(SubroutineBodyNode* node)  {
		s_stackCurSubroutineZone.push(node);
	}
	static void quitSubRoutineBodyZone()  {
		assert(s_stackCurSubroutineZone.size() > 0);
		s_stackCurSubroutineZone.pop();
	}


	static stack<CompondStatement*> s_stackCurCompoundStatmentZone;       //��ǰ�������������
	static CompondStatement* getCurCompoundStatmentNode()  {
		if (s_stackCurCompoundStatmentZone.size() == 0)  {
			return nullptr;
		}
		return s_stackCurCompoundStatmentZone.top();
	}
	static void insertCompoundStatmentNode(CompondStatement* node)  {
		s_stackCurCompoundStatmentZone.push(node);
	}
	static void quitCompoundStatmentZone()  {
		assert(s_stackCurCompoundStatmentZone.size() > 0);
		s_stackCurCompoundStatmentZone.pop();
	}
};




typedef GramTreeNodeBase TreeNode;

class TreeNodeList
{
	TreeNode* _head;
	TreeNode* _cur;
public:
	TreeNodeList()  {
		_head = _cur = nullptr;
	}
	void Push(TreeNode* node)  {     //�������һ������....
		if (node != nullptr)  {
			if (_head == nullptr)  {
				TreeNode* curNode = getCurNode(node);
				if (curNode != node)  {  //Ҫ����Ľڵ��Ǹ����ڵ���Ҫ��ɢһ��һ���ļ�
					_head = node;
					_cur = curNode;
				}
				else  {
					_head = _cur = node;
				}
			}
			else  {
				TreeNode* curNode = getCurNode(node);  //�ڵ�ĵ�ǰ�ڵ�,�����һ���ڵ�
				if (curNode != node)  {                //Ҫ����Ľڵ��Ǹ����ڵ���Ҫ��ɢһ��һ���ļ�
					_cur->setNextNode(node);
					_cur = curNode;
				}
				else  {
					_cur->setNextNode(node);
					_cur = node;
				}
			}
		}
	}
	TreeNode* getHeadNode()  {
		return _head;
	}
	TreeNode* getCurNode()  {
		return _cur;
	}
	static TreeNode* getCurNode(TreeNode* node)  {
		TreeNode* curNode = nullptr;
		while (node)  {
			curNode = node;
			node = node->getNextNode();
		}
		return curNode;
	}
};





class ClassTreeNode : public GramTreeNodeBase  {   //�����ͽڵ�,�������͵�����parentָ�����
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

class SubroutineDecNode : public GramTreeNodeBase  {     //���������Ľڵ�
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


class SubroutineBodyNode : public GramTreeNodeBase  {    //������ڵ�
public:
	SubroutineBodyNode() : GramTreeNodeBase()  {
		_nodeKind = SUBROUTINE_BODY_K;
		insertSubRoutineBodyNode(this);
	}
	virtual ~SubroutineBodyNode(){}

	enum SubroutineBody  {
		VarDec = 0,
		Statement
	};

	TreeNodeList _statementList;
	TreeNodeList _varDecList;
	void addStatement(GramTreeNodeBase* node)  {
		if (getCurCompoundStatmentNode() == nullptr)  {     //û�и������(if,while��)����������,��Ϊ��if����л���Ϊ����䱻��ӵ�
			_statementList.Push(node);
		}
	}
	void addVarDec(GramTreeNodeBase* node)  {
		_varDecList.Push(node);
	}
	void addBodyChild()  {
		GramTreeNodeBase::addChild(_varDecList.getHeadNode(), VarDec);
		GramTreeNodeBase::addChild(_statementList.getHeadNode(), Statement);
	}

	int getFuncLocalsNum();
};


class AssignStatement : public GramTreeNodeBase  {    //��ֵ���ڵ�
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


class CompondStatement : public GramTreeNodeBase  {    //��ֵ���ڵ�
public:
	CompondStatement(int nK) : GramTreeNodeBase(nK)  {
		insertCompoundStatmentNode(this);
	}
	virtual ~CompondStatement(){}

};




class VarDecNode : public GramTreeNodeBase  {   //���������ڵ�
public:
	VarDecNode() : GramTreeNodeBase()  {
		_nodeKind = VAR_DEC_K;
	}
	enum EVarDec  {
		VarDec_Type = 0,
		VarDec_Name
	};
	virtual ~VarDecNode(){}
	GramTreeNodeBase* getVarDecType();    //����������
	GramTreeNodeBase* getVarDecName();    //����������

};




class ParamNode : public VarDecNode  {    //�βνڵ�
public:
	ParamNode() : VarDecNode()  {
		_nodeKind = PARAM_K;
	}
	virtual ~ParamNode(){}
};

