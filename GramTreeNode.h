#pragma once
#include <string>
#include <map>
#include <stack>
#include <cassert>
using std::string;
using std::stack;

#include "Scanner.h"

class SubroutineBodyNode;
class SubroutineDecNode;
class CompondStatement;
class ClassTreeNode;

class GramTreeNodeBase
{
public:
	static const int Child_Num_Const = 5;
	static int s_nCurNodeIndex;
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

		_nodeIndex = ++s_nCurNodeIndex;
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
	int      _nodeIndex;      //��ǰ�ڵ��������,�����жϱ����Ƿ�������֮ǰʹ��
	  
	Scanner::Token _token;

public:
	void setToken(Scanner::Token& t)  {
		_token = t;
	}
	const string getLexeme()  {
		return _token.lexeme;
	}
	void setLexeme(string& str)  {
		_token.lexeme = str;
	}
	unsigned int getRow()  {
		return _token.row;
	}
	void setNodeKind(int kind)  {
		_nodeKind = (NodeKind)kind;
	}
	NodeKind getNodeKind()  {
		return _nodeKind;
	}
	int getNodeIndex()  {
		return _nodeIndex;
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
	void setParentNode(GramTreeNodeBase* node)  {   //ҲҪ�����ֵ�ָ������
		auto cur = this;
		while (cur)  {
			cur->_pParent = node;
			cur = cur->getNextNode();
		}
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

	virtual GramTreeNodeBase* clone() {
		auto node = new GramTreeNodeBase;
		node->_nodeKind = _nodeKind;
		node->_token = _token;
		node->_strClassName = _strClassName;
		node->_pParent = _pParent;
		node->_strNodeDes = _strNodeDes;
		node->_childIndex = _childIndex;
		node->_siblings = _siblings;
		return  node;
	}


	static GramTreeNodeBase* s_curVarDecType;                                  //��ǰ����������,���ڽ���int a=1,b=2;b������

	static stack<ClassTreeNode*> s_stackCurClassZone;                        //��ǰ���������
	static ClassTreeNode* getCurCurClassNode();
	static void insertClassNode(ClassTreeNode* node);
	static void quitClassZone();


	static stack<SubroutineDecNode*> s_stackCurSubroutineZone;               //��ǰ�ĺ���������
	static SubroutineDecNode* getCurSubroutineNode();
	static void insertSubRoutineNode(SubroutineDecNode* node);
	static void quitSubRoutineZone();

	static stack<SubroutineBodyNode*> s_stackCurSubroutineBodyZone;          //��ǰ�ĺ�����������
	static SubroutineBodyNode* getCurSubroutineBodyNode();
	static void insertSubRoutineBodyNode(SubroutineBodyNode* node);
	static void quitSubRoutineBodyZone();


	static stack<CompondStatement*> s_stackCurCompoundStatmentZone;          //��ǰ�������������
	static CompondStatement* getCurCompoundStatmentNode(int* pNum = 0);  
	static void insertCompoundStatmentNode(CompondStatement* node);
	static void quitCompoundStatmentZone();
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
		insertClassNode(this);
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

	bool hasVarDecInField(GramTreeNodeBase* node);       //�����ı����Ƿ���������б���

};


class SubroutineBodyNode;

class SubroutineDecNode : public GramTreeNodeBase  {     //���������Ľڵ�
public:
	SubroutineDecNode() : GramTreeNodeBase()  {
		_nodeKind = SUBROUTINE_DEC_K;
		insertSubRoutineNode(this);
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

	bool hasVarDecInParams(GramTreeNodeBase* node);       //�����ı����Ƿ��ڲ����б���
};

class VarDecNode;

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
	bool hasVarDec(GramTreeNodeBase* node);
	VarDecNode* getCurVarDec();

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

