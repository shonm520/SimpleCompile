#ifndef _PARSER_H
#define _PARSER_H

#include "Scanner.h"
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <queue>
#include "Error.h"

using namespace std;

class Parser
{
public:
    enum NodeKind {
        None, CLASS_K, CLASS_VAR_DEC_K, SUBROUTINE_DEC_K, BASIC_TYPE_K, CLASS_TYPE_K, NULL_K,
        PARAM_K, VAR_DEC_K, ARRAY_K, VAR_K, IF_STATEMENT_K, WHILE_STATEMENT_K, CALL_EXPRESSION_K,
        RETURN_STATEMENT_K, CALL_STATEMENT_K, BOOL_EXPRESSION_K,FUNCTION_CALL_K,CONSTRUCTOR_CALL_K,
        COMPARE_K, OPERATION_K, BOOL_K, ASSIGN_K, SUBROUTINE_BODY_K, BOOL_CONST_K, NEGATIVE_K,METHOD_CALL_K,
        INT_CONST_K, CHAR_CONST_K, STRING_CONST_K, KEY_WORD_CONST, THIS_K
    };
	static string Kind2Des(NodeKind nK)  {
		static map<NodeKind, string> mapStrDes;
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
    class TreeNode
    {
	public:
		static const int Child_Num_Const = 5;
	private:
		TreeNode *child_[Child_Num_Const];
		//vector<TreeNode*> _vtTreeNodes;
        TreeNode *next;
        NodeKind nodeKind;
		string   strNodeDes;
		int    childIndex;   //���������
		TreeNode* parent;

		enum  SubroutineFiled  {
			Sign = 0,
			Ret,
			Name,
			Params,
			Body
		};
		enum SubroutineBody  {
			VarDec = 0,
			Statement
		};
		Scanner::Token token;
	public:
		//Scanner::Token token;
		TreeNode(NodeKind nK = None)
        {
			nodeKind = nK;
            //child[0] = child[1] = child[2] = child[3] = child[4] = nullptr;
            next = nullptr;
			strNodeDes = Kind2Des(nK);
			childIndex = -1;
			parent = nullptr;
			memset(&child_, 0, sizeof(TreeNode*) * Child_Num_Const);
        }
		void SetToken(Scanner::Token& t)  {
			token = t;
		}
		const string GetLexeme()  {
			return token.lexeme;
		}
		void SetLexeme(string& str)  {
			token.lexeme = str;
		}
		unsigned int GetRow()  {
			return token.row;
		}
		void SetNodeKind(NodeKind kind)  {
			nodeKind = kind;
		}
		NodeKind GetNodeKind()  {
			return nodeKind;
		}
		void SetNextNode(TreeNode* node)  {
			next = node;
		}
		TreeNode* GetNextNode()  {
			return next;
		}
		void AddChild(TreeNode* pChild, int ind = -1)  {
			if (pChild)   {
				//_vtTreeNodes.push_back(pChild);
				if (ind >= 0)  {
					child_[ind] = pChild;
				}
				pChild->childIndex = ind;
				pChild->parent = this;
			}
		}
		TreeNode* GetChildByIndex(int ind)  {
			//return _vtTreeNodes[ind];
			return child_[ind];
		}

		TreeNode* GetChild_SubroutineSign()  {    //��־,method,function,constructor
			if (this->nodeKind == Parser::SUBROUTINE_DEC_K)  {
				return child_[SubroutineFiled::Sign];
			}
			return nullptr;
		}

		TreeNode* GetChild_SubroutineName()  {
			if (this->nodeKind == Parser::SUBROUTINE_DEC_K)  {
				return child_[SubroutineFiled::Name];
			}
			return nullptr;
		}

		TreeNode* GetChild_SubroutineBody()  {
			if (this->nodeKind == Parser::SUBROUTINE_DEC_K)  {
				return child_[SubroutineFiled::Body];
			}
			return nullptr;
		}
		TreeNode* GetChild_SubroutineBody_VarDec()  {
			if (this->nodeKind == Parser::SUBROUTINE_BODY_K)  {
				return child_[SubroutineBody::VarDec];
			}
			return nullptr;
		}

		TreeNode* GetChild_AssignVarName()  {  //��ֵ���ı���
			if (this->nodeKind == Parser::ASSIGN_K)  {
				return child_[0];
			}
			return nullptr;
		}

		TreeNode* GetChildByTag(string tag)  {
			if (this->nodeKind == Parser::ASSIGN_K)  {
				if (tag == "var_name")  {  //��ֵʱ�ı�����
					return child_[0];
				}
				else if (tag == "var_rval")  { //��ֵʱ�ı������ʽ����ֵ  
					return child_[1];
				}
			}
			return nullptr;
		}
		
		int GetFuncLocalsNum()  {  //��ȡ�����ֲ������ĸ���
			int nlocals = 0;
			if (this->nodeKind == Parser::SUBROUTINE_DEC_K)  {
				for (auto p = GetChild_SubroutineBody()->GetChild_SubroutineBody_VarDec(); p != nullptr; p = p->next)  {
					for (auto q = p->GetChildByIndex(1); q != nullptr; q = q->next)  {
						nlocals++;
					}
				}
				return nlocals;
			}
			else if (this->nodeKind == Parser::SUBROUTINE_BODY_K)  {
				for (auto q = GetChildByIndex(1); q != nullptr; q = q->next)  {
					nlocals++;
				}
				return nlocals;
			}

			return 0;
		}
		
    };
	class TreeNodeList
	{
		TreeNode* _head;
		TreeNode* _cur;
	public:
		TreeNodeList()  {
			_head = _cur =  nullptr;
		}
		void Push(TreeNode* node)  {
			if (node != nullptr)  {
				if (_head == nullptr)  {
					_head = _cur = node;
				}
				else  {
					_cur->SetNextNode(node);
					_cur = node;
				}
			}
		}
		TreeNode* GetHeadNode()  {
			return _head;
		}
	}; 

	struct CirQueue
	{
		CirQueue()  {
			_index = 0;
			_front = 0;
		}
		int _index;
		int _front;
		static const int Cap = 20;
		Scanner::Token _member[Cap];
		static int Add(int n, int step = 1)  {
			n = (n + Cap + step) % Cap;
			return n;
		}
		void Push(Scanner::Token& node)  {
			_member[_index] = node;
		}
		void Back(int step)  {
			_index = Add(_index, step * -1);
		}
		bool GetItemFromNew()  {
			return _index == _front;
		}
		Scanner::Token GetFront() {
			int ind = _index;
			if (_index == _front)   {
				_front = Add(_front);
			}
			_index = Add(_index);
			return _member[ind];
		}
	};
private:
    vector<string> _vtfilenames;
    string _strCurParserFileName;
    TreeNode *_pSyntaxTree;
    Scanner _scanner;
    bool _hasRetStatement;                           // Ҫ��֤ÿ����������return���, ��ʹ����ֵΪvoid

    Scanner::Token getToken();                          // �ӻ�������ȡ��һ��token
    Scanner::Token ungetToken();                        // ����һ��ȡ����token���뵽��������
    deque<Scanner::Token> tokenBuffer1;                 // �󻺳���
    deque<Scanner::Token> tokenBuffer2;                 // �һ�����
	CirQueue _cirQueue;
    string getFullName(string name);                    // ����

    TreeNode * parse_class_list();
    TreeNode * parse_class();
    TreeNode * parse_class_var_dec_list();
    TreeNode * parse_class_var_dec();
    TreeNode * parse_var_name_list();
    TreeNode * parse_type();
    TreeNode * parse_subroutine_dec_list();
    TreeNode * parse_subroutin_dec();
    TreeNode * parse_params();
    TreeNode * parse_param_list();
    TreeNode * parse_param();
    TreeNode * parse_subroutine_body();
    TreeNode * parse_var_dec_list();
    TreeNode * parse_var_dec();
    TreeNode * parse_statements();
    TreeNode * parse_statement();
    TreeNode * parse_assign_statement();
    TreeNode * parse_left_value();
    TreeNode * parse_if_statement();
    TreeNode * parse_while_statement();
    TreeNode * parse_return_statement();
    TreeNode * parse_call_statement();
    TreeNode * parse_expressions();
    TreeNode * parse_expression_list();
    TreeNode * parse_expression();
    TreeNode * parse_bool_expression();
    TreeNode * parse_additive_expression();
    TreeNode * parse_term();
    TreeNode * parse_factor();
    TreeNode * parse_positive_factor();
    TreeNode * parse_not_factor();
    TreeNode * parse_call_expression();

    void printSyntaxTree(TreeNode *tree, int dep = 1);
public:
    Parser(vector<string> & filenames);
    bool hasError();
    TreeNode *getSyntaxTree();
    void print();
    void parse_program();
    static string getCallerName(string fullName);
    static string getFunctionName(string fullName);
};

#endif
