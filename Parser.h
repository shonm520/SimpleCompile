#ifndef _PARSER_H
#define _PARSER_H

#include "Scanner.h"
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <queue>
#include "Error.h"
#include "GramTreeNode.h"

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
	/*static string Kind2Des(NodeKind nK)  {
		static map<NodeKind, string> mapStrDes;
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
	}*/

 	typedef GramTreeNodeBase TreeNode;

	struct CirQueue
	{
		CirQueue()  {
			_index = 0;
			_front = 0;
			
			_pScanner = nullptr;
			_top = 0;
			_cur = -1;
			_peek = -1;
		}
		int _index;
		int _front;
		int _peek;
		int _top, _cur;
		Scanner* _pScanner;
		static const int Cap = 20;
		Scanner::Token _member[Cap];
		void SetScanner(Scanner* ps)  {
			_pScanner = ps;
		}
		static int Add(int n, int step = 1)  {
			n = (n + Cap + step) % Cap;
			return n;
		}
// 		void Push(Scanner::Token& node)  {
// 			_member[_front] = node;
// 			_pScanner->nextToken();
// 		}
// 		void Back(int step)  {
// 			_index = Add(_index, step * -1);
// 		}
// 		bool GetItemFromNew()  {
// 			return _index == _front;
// 		}
// 		Scanner::Token GetCurrent() {
// 			int ind = _index;
// 			if (_index == _front)   {
// 				_front = Add(_front);
// 			}
// 			_index = Add(_index);
// 			return _member[ind];
// 		}
// 
// 		Scanner::Token Peek()  {
// 			_peek = _peek + 1;
// 			if (_peek > _front)   {
// 				_front = Add(_front);
// 			}
// 			return _member[_peek];
// 		}


		static void _add(int& n, int step = 1)  {
			n = (n + Cap + step) % Cap;
		}

		static bool _eq(int cur, int top)  {
			return cur == top;
		}

		Scanner::Token getCurrent()  {
			_add(_cur);
			if (_eq(_cur, _top))  {
				Scanner::Token token = _pScanner->nextToken();
				_member[_top] = token;
				_add(_top);
			}
			resetPeek();
			return _member[_cur];
		}

		Scanner::Token peekToken(bool reset)  {
			if (reset)  {
				resetPeek();
			}
			_add(_peek);
			if (_eq(_peek, _top))  {
				Scanner::Token token = _pScanner->nextToken();
				_member[_top] = token;
				_add(_top);
			}
			return _member[_peek];
		}

		void resetPeek()  {
			_peek = _cur;
		}
		void back(int step = 1)  {
			_add(_cur, step * -1);
		}




	};
private:
    vector<string> _vtfilenames;
    string _strCurParserFileName;
    TreeNode *_pSyntaxTree;
    Scanner _scanner;
    bool _hasRetStatement;                              // 要保证每个函数都有return语句, 即使返回值为void

    Scanner::Token getToken();                          // 从缓冲区中取出一个token
    void ungetToken();                        // 把上一次取出的token放入到缓冲区中
	bool eatExpectedToken(string expected, Scanner::Token* pToken = nullptr);
	bool eatExpectedToken(Scanner::TokenType type, Scanner::Token* pToken = nullptr);

	Scanner::Token peekToken(bool reset);
	//void resetPeek();

	CirQueue _cirQueue;
    string getFullName(string name);                    // 返回

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
	TreeNode * parse_var_dec_list(TreeNodeList& statementNodeList);
	TreeNode * parse_var_dec(TreeNodeList& statementNodeList);
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
	static bool isBasicType(string);    //是否是基本类型
};

#endif
