#include "GramTreeNode.h"
#include "Parser.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

Parser::Parser(vector<string> &filenames)
{
	this->_vtfilenames = filenames;
	_pSyntaxTree = nullptr;
}

Scanner::Token Parser::getToken()
{
	if (_cirQueue.GetItemFromNew())  {
		auto token = _scanner.nextToken();
		_cirQueue.Push(token);
	}
	return _cirQueue.GetFront();
}

Scanner::Token Parser::ungetToken()
{
	_cirQueue.Back(1);
	return _cirQueue._member[_cirQueue._index];
}

string Parser::getFullName(string name)
{
	string fullname = _strCurParserFileName + "." + name;
	return fullname;
}

string Parser::getCallerName(string fullName)
{
	auto iter = fullName.cbegin();
	while (iter != fullName.cend()) {
		if (*iter == '.')
			break;
		++iter;
	}
	return string(fullName.cbegin(), iter);
}

string Parser::getFunctionName(string fullName)
{
	auto iter = fullName.cbegin();
	while (iter != fullName.cend()) {
		if (*iter == '.')
			break;
		++iter;
	}
	return string(++iter, fullName.cend());
}

void Parser::parse_program()
{
	_pSyntaxTree = parse_class_list();
}

Parser::TreeNode *Parser::parse_class_list()
{
	TreeNode *p = nullptr;
	TreeNodeList nodeList;
	for (auto filenameIter = _vtfilenames.cbegin(); filenameIter != _vtfilenames.cend(); ++filenameIter) {
		int ret = _scanner.openFile(*filenameIter);
		if (ret != 0) continue;
		auto classNameIter = filenameIter->rbegin();
		int begin = 0;
		while (classNameIter != filenameIter->rend()) {
			if (*classNameIter == '/')
				break;
			begin++;
			++classNameIter;
		}
		_strCurParserFileName = filenameIter->substr(filenameIter->size() - begin, begin - 5);
		_scanner.resetRow();
		TreeNode *q = parse_class();
		if (getToken().kind != Scanner::ENDOFFILE)
			cerr << "Syntax Error in class " << _strCurParserFileName << ": unexpected token before EOF " << endl;
		nodeList.Push(q);
		_scanner.closeFile();
	}
	return nodeList.GetHeadNode();
}

Parser::TreeNode *Parser::parse_class()
{
	TreeNode *t = new ClassTreeNode();
	Scanner::Token token = getToken();   //class
	token = getToken();    //class_name Main
	if (token.kind != Scanner::ID) {
		syntaxError(_strCurParserFileName, "identifier", token);
		return t;
	}
	auto node = new TreeNode;
	node->setToken(token);
	t->addChild(node, 0);
	if (_strCurParserFileName != token.lexeme) {
		error1(_strCurParserFileName);
		return t;
	}
	token = getToken();    // {
	if (token.lexeme != "{") {
		syntaxError(_strCurParserFileName, "{", token);
		return t;
	}
	t->addChild(parse_class_var_dec_list(), 1);
	t->addChild(parse_subroutine_dec_list(), 2);
	token = getToken();
	if (token.lexeme != "}") {
		syntaxError(_strCurParserFileName, "}", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_class_var_dec_list()
{
	auto token = getToken();
	TreeNodeList nodeList;
	while (token.lexeme == "static" || token.lexeme == "field") {
		ungetToken();
		TreeNode *q = parse_class_var_dec();
		nodeList.Push(q);
		token = getToken();
	}
	ungetToken();
	return nodeList.GetHeadNode();
}

Parser::TreeNode *Parser::parse_class_var_dec()
{
	TreeNode *t = new TreeNode(CLASS_VAR_DEC_K);
	t->setNodeKind(CLASS_VAR_DEC_K);
	Scanner::Token token = getToken();
	if (token.lexeme != "static" && token.lexeme != "field") {
		syntaxError(_strCurParserFileName, "static or filed", token);
		return t;
	}
	auto node = new TreeNode;
	node->setLexeme(token.lexeme);
	t->addChild(node, 0);
	t->addChild(parse_type(), 1);
	t->addChild(parse_var_name_list(), 2);
	token = getToken();
	if (token.lexeme != ";") {
		syntaxError(_strCurParserFileName, ";", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_var_name_list()
{
	TreeNode *t = new TreeNode;
	Scanner::Token token = getToken();
	if (token.kind != Scanner::ID) {
		syntaxError(_strCurParserFileName, "identifier", token);
		return t;
	}
	t->setToken(token);
	TreeNode *p = t;
	token = getToken();
	while (token.lexeme == ",") {
		token = getToken();
		if (token.kind != Scanner::ID) {
			syntaxError(_strCurParserFileName, "identifier", token);
			return t;
		}
		TreeNode *q = new TreeNode;
		q->setToken(token);
		p->setNextNode(q);
		p = q;
		token = getToken();
	}
	ungetToken();
	return t;
}

Parser::TreeNode *Parser::parse_type()
{
	TreeNode *t = nullptr;
	Scanner::Token token = getToken();
	if (token.kind == Scanner::ID) {
		t = new TreeNode(CLASS_TYPE_K);
		t->setNodeKind(CLASS_TYPE_K);
		t->setLexeme(token.lexeme);
	}
	else if (token.lexeme == "int" ||
		     token.lexeme == "char" ||
			 token.lexeme == "boolean" ||
			 token.lexeme == "void")  {
		t = new TreeNode(BASIC_TYPE_K);
		t->setLexeme(token.lexeme);
	}
	else {
		syntaxError(_strCurParserFileName, "basic type or class type", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_subroutine_dec_list()
{
	auto token = getToken();
	TreeNodeList nodeList;
	while (token.lexeme == "constructor" ||
		   token.lexeme == "function" || 
		   token.lexeme == "method") {
		ungetToken();
		TreeNode *q = parse_subroutin_dec();    //解析其中一个函数
		
		nodeList.Push(q);
		
		token = getToken();
	}
	ungetToken();
	return nodeList.GetHeadNode();
}

Parser::TreeNode *Parser::parse_subroutin_dec()
{
	TreeNode *t = new SubroutineDecNode();

	Scanner::Token token = getToken();
	if (token.lexeme == "constructor" ||
		token.lexeme == "function" ||
		token.lexeme == "method")  {
		auto node = new TreeNode();
		node->setToken(token);
		t->addChild(node, 0);
	}
	else {
		syntaxError(_strCurParserFileName, "constructor or function or method", token);
		return t;
	}
	t->addChild(parse_type(), 1);
	token = getToken();
	if (token.kind == Scanner::ID) {
		auto node = new TreeNode();
		node->setToken(token);
		node->setLexeme(getFullName(token.lexeme));
		t->addChild(node, 2);
	}
	else {
		syntaxError(_strCurParserFileName, "identifile", token);
		return t;
	}

	token = getToken();
	if (token.lexeme != "(") {
		syntaxError(_strCurParserFileName, "(", token);
		return t;
	}
	t->addChild(parse_params(), 3);
	token = getToken();
	if (token.lexeme != ")") {
		syntaxError(_strCurParserFileName, ")", token);
		return t;
	}
	t->addChild(parse_subroutine_body(), 4);
	return t;
}

Parser::TreeNode *Parser::parse_params()
{
	TreeNode *t = nullptr;
	Scanner::Token token = getToken();
	if (token.lexeme != ")") {
		ungetToken();
		t = parse_param_list();
	}
	else
		ungetToken();
	return t;
}

Parser::TreeNode *Parser::parse_param_list()
{
	TreeNode *t = parse_param();
	TreeNodeList nodeList;
	nodeList.Push(t);
	Scanner::Token token = getToken();
	while (token.lexeme == ",") {
		TreeNode *q = parse_param();
		nodeList.Push(q);
		token = getToken();
	}
	ungetToken();
	return nodeList.GetHeadNode();
}

Parser::TreeNode *Parser::parse_param()
{
	TreeNode *t = new TreeNode(PARAM_K);
	t->setNodeKind(PARAM_K);
	t->addChild(parse_type(), 0);                 //类型
	Scanner::Token token = getToken();
	if (token.kind == Scanner::ID) {
		auto node = new TreeNode;;
		node->setToken(token);
		t->addChild(node, 1);                    //值
	}
	else {
		syntaxError(_strCurParserFileName, "identifier", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_subroutine_body()
{
	_hasRetStatement = false;

	TreeNode *t = new TreeNode(SUBROUTINE_BODY_K);
	t->setNodeKind(SUBROUTINE_BODY_K);
	Scanner::Token token = getToken();
	if (token.lexeme != "{") {
		syntaxError(_strCurParserFileName, "{", token);
		return t;
	}
	t->addChild(parse_var_dec_list(), 0);
	t->addChild(parse_statements(), 1);

	token = getToken();
	if (token.lexeme != "}") {
		syntaxError(_strCurParserFileName, "}", token);
		return t;
	}
	if (_hasRetStatement == false) {
		syntaxError(_strCurParserFileName, "return statement", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_var_dec_list()
{
	TreeNode *t = nullptr;
	TreeNode *p = t;

	TreeNodeList nodeList;

Loop:
	Scanner::Token token = getToken();
	if (token.lexeme == "int" || token.lexeme == "char"
		|| token.lexeme == "boolean" || token.lexeme == "string") {
		ungetToken();
		TreeNode *q = parse_var_dec();
		nodeList.Push(q);
		goto Loop;
	}
	else if (token.kind == Scanner::ID) { // 类变量声明
		token = getToken();
		if (token.kind == Scanner::ID) {
			ungetToken();
			ungetToken();
			TreeNode *q = parse_var_dec();
			nodeList.Push(q);
			goto Loop;
		}
		ungetToken();
	}
	ungetToken();
	//return t;
	return nodeList.GetHeadNode();
}

Parser::TreeNode *Parser::parse_var_dec()
{
	TreeNode *t = new TreeNode(VAR_DEC_K);
	t->setNodeKind(VAR_DEC_K);
	Scanner::Token token;
	//t->child[0] = parse_type();
	//t->child[1] = parse_var_name_list();
	t->addChild(parse_type(), 0);
	t->addChild(parse_var_name_list(), 1);
	token = getToken();
	if (token.lexeme != ";") {
		syntaxError(_strCurParserFileName, ";", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_statements()
{
	TreeNode *t = nullptr;
	TreeNode *p = t;
	Scanner::Token token = getToken();
	while (token.lexeme == "if" || token.lexeme == "while" || token.lexeme == "return"
		|| token.kind == Scanner::ID) {
		if (token.lexeme == "return")
			_hasRetStatement = true;
		if (token.kind == Scanner::ID) {
			token = getToken();
			if (token.lexeme == "=" || token.lexeme == "[" || token.lexeme == "(" || token.lexeme == ".") {
				ungetToken();
				ungetToken();
				TreeNode *q = parse_statement();
				if (t == nullptr)
					t = p = q;
				else {
					p->setNextNode(q);
					p = q;
				}
			}
			else {
				ungetToken();
				break;
			}
		}
		else {
			ungetToken();
			TreeNode *q = parse_statement();
			if (t == nullptr)
				t = p = q;
			else {
				p->setNextNode(q);
				p = q;
			}
		}
		token = getToken();
	}
	ungetToken();

	return t;
}

Parser::TreeNode *Parser::parse_statement()
{
	TreeNode *t = nullptr;
	Scanner::Token token = getToken();
	if (token.lexeme == "if") {
		ungetToken();
		t = parse_if_statement();
	}
	else if (token.lexeme == "while") {
		ungetToken();
		t = parse_while_statement();
	}
	else if (token.lexeme == "return") {
		ungetToken();
		t = parse_return_statement();
	}
	else if (token.kind == Scanner::ID) {
		token = getToken();
		if (token.lexeme == "=" || token.lexeme == "[") {
			ungetToken();
			ungetToken();
			t = parse_assign_statement();

		}
		else if (token.lexeme == "(" || token.lexeme == ".") {
			ungetToken();
			ungetToken();
			t = parse_call_statement();
			token = getToken();
			if (token.lexeme != ";") {
				ungetToken();
				syntaxError(_strCurParserFileName, ";", token);
				return t;
			}
		}
		else {
			ungetToken();
			ungetToken();
			syntaxError(_strCurParserFileName, "'=' or '[' or '(' or '.'", token);
			return t;
		}
	}
	else {
		ungetToken();
		syntaxError(_strCurParserFileName, "identifier", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_assign_statement()
{
	TreeNode *t = new AssignStatement();
	t->addChild(parse_left_value(), 0);
	Scanner::Token token = getToken();
	t->addChild(parse_expression(), 1);
	token = getToken();
	if (token.lexeme != ";") {
		syntaxError(_strCurParserFileName, ";", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_left_value()
{
	TreeNode *t = new TreeNode(VAR_K);
	t->setNodeKind(VAR_K);
	Scanner::Token token = getToken();
	t->setToken(token);
	token = getToken();
	if (token.lexeme == "[") {
		t->setNodeKind(ARRAY_K);
		//t->child[0] = parse_expression();
		t->addChild(parse_expression(), 0);
		token = getToken();
		if (token.lexeme != "]") {
			syntaxError(_strCurParserFileName, "]", token);
			return t;
		}
		t->setNodeKind(ARRAY_K);
	}
	else if (token.lexeme == "=") {
		ungetToken();
	}
	return t;
}

Parser::TreeNode *Parser::parse_if_statement()
{
	TreeNode *t = new TreeNode(IF_STATEMENT_K);
	t->setNodeKind(IF_STATEMENT_K);
	Scanner::Token token = getToken();
	token = getToken();
	if (token.lexeme != "(") {
		syntaxError(_strCurParserFileName, "(", token);
		return t;
	}
	//t->child[0] = parse_expression();
	t->addChild(parse_expression(), 0);
	token = getToken();
	if (token.lexeme != ")") {
		syntaxError(_strCurParserFileName, ")", token);
		return t;
	}
	token = getToken();
	if (token.lexeme != "{") {
		syntaxError(_strCurParserFileName, "{", token);
		return t;
	}
	//t->child[1] = parse_statements();
	t->addChild(parse_statements(), 1);
	token = getToken();
	if (token.lexeme != "}") {
		syntaxError(_strCurParserFileName, "}", token);
		return t;
	}
	token = getToken();
	if (token.lexeme == "else") {
		token = getToken();
		if (token.lexeme != "{") {
			syntaxError(_strCurParserFileName, "{", token);
			return t;
		}
		//t->child[2] = parse_statements();
		t->addChild(parse_statements(), 2);
		token = getToken();
		if (token.lexeme != "}") {
			syntaxError(_strCurParserFileName, "}", token);
			return t;
		}
	}
	else
		ungetToken();
	return t;
}

Parser::TreeNode *Parser::parse_while_statement()
{
	TreeNode *t = new TreeNode(WHILE_STATEMENT_K);
	t->setNodeKind(WHILE_STATEMENT_K);
	Scanner::Token token = getToken();
	token = getToken();
	if (token.lexeme != "(") {
		syntaxError(_strCurParserFileName, "(", token);
		return t;
	}
	//t->child[0] = parse_expression();
	t->addChild(parse_expression(), 0);
	token = getToken();
	if (token.lexeme != ")") {
		syntaxError(_strCurParserFileName, ")", token);
		return t;
	}
	token = getToken();
	if (token.lexeme != "{") {
		syntaxError(_strCurParserFileName, "{", token);
		return t;
	}
	//t->child[1] = parse_statements();
	t->addChild(parse_statements(), 1);
	token = getToken();
	if (token.lexeme != "}") {
		syntaxError(_strCurParserFileName, "}", token);
		return t;
	}
	return t;
}

Parser::TreeNode *Parser::parse_return_statement()
{
	TreeNode *t = new TreeNode(RETURN_STATEMENT_K);
	t->setNodeKind(RETURN_STATEMENT_K);
	Scanner::Token token = getToken();
	t->setToken(token);
	token = getToken();
	if (token.lexeme == ";")
		return t;
	else {
		ungetToken();
		//t->child[0] = parse_expression();
		t->addChild(parse_expression(), 0);
		token = getToken();
		if (token.lexeme != ";") {
			syntaxError(_strCurParserFileName, ";", token);
			return t;
		}
	}
	return t;
}

Parser::TreeNode *Parser::parse_call_statement()
{
	TreeNode *t = new TreeNode(CALL_STATEMENT_K);
	t->setNodeKind(CALL_STATEMENT_K);
	Scanner::Token token = getToken();
	Scanner::Token save = token;
	//t->child[0] = new TreeNode;
	auto node = new TreeNode;
	t->addChild(node, 0);
	token = getToken();
	if (token.lexeme == "(") {
		node->setNextNode(parse_expressions());
		token = getToken();
		if (token.lexeme != ")") {
			syntaxError(_strCurParserFileName, ")", token);
			return t;
		}
	}
	else if (token.lexeme == ".") {
		token = getToken();
		if (token.kind != Scanner::ID) {
			syntaxError(_strCurParserFileName, "identifier", token);
			return t;
		}
		save.lexeme = save.lexeme + "." + token.lexeme;
		token = getToken();
		if (token.lexeme != "(") {
			syntaxError(_strCurParserFileName, "(", token);
			return t;
		}
		node->setNextNode(parse_expressions());
		token = getToken();
		if (token.lexeme != ")") {
			syntaxError(_strCurParserFileName, ")", token);
			return t;
		}
	}
	t->setToken(save);
	return t;
}

Parser::TreeNode *Parser::parse_expressions()
{
	TreeNode *t = nullptr;
	Scanner::Token token = getToken();
	if (token.lexeme == ")") {
		ungetToken();
		return t;
	}
	else {
		ungetToken();
		t = parse_expression_list();
		return t;
	}
}

Parser::TreeNode *Parser::parse_expression_list()
{
	TreeNode *t = parse_expression();
	TreeNode *p = t;
	Scanner::Token token = getToken();
	while (token.lexeme == ",") {
		TreeNode *q = parse_expression();
		p->setNextNode(q);
		p = q;
		token = getToken();
	}
	ungetToken();
	return t;
}

Parser::TreeNode *Parser::parse_expression()
{
	TreeNode *t = parse_bool_expression();
	Scanner::Token token = getToken();
	while (token.lexeme == "&" || token.lexeme == "|") {
		TreeNode *p = new TreeNode(BOOL_EXPRESSION_K);
		p->setNodeKind(BOOL_EXPRESSION_K);
		p->setToken(token);
		//p->child[0] = t;
		p->addChild(t, 0);
		t = p;
		//t->child[1] = parse_bool_expression();
		t->addChild(parse_bool_expression(), 1);
		token = getToken();
	}
	ungetToken();
	return t;
}

Parser::TreeNode *Parser::parse_bool_expression()
{
	TreeNode *t = parse_additive_expression();
	Scanner::Token token = getToken();
	if (token.lexeme == "<=" || token.lexeme == ">=" || token.lexeme == "=="
		|| token.lexeme == "<" || token.lexeme == ">" || token.lexeme == "!=") {
		TreeNode *p = new TreeNode;
		p->setNodeKind(COMPARE_K);
		p->setToken(token);
		//p->child[0] = t;
		p->addChild(t, 0);
		t = p;
		//t->child[1] = parse_additive_expression();
		t->addChild(parse_additive_expression(), 1);
	}
	else
		ungetToken();
	return t;
}

Parser::TreeNode *Parser::parse_additive_expression()
{
	TreeNode *t = parse_term();
	Scanner::Token token = getToken();
	while (token.lexeme == "+" || token.lexeme == "-") {
		TreeNode *p = new TreeNode(OPERATION_K);
		p->setNodeKind(OPERATION_K);
		p->setToken(token);
		//p->child[0] = t;
		p->addChild(t, 0);
		t = p;
		//p->child[1] = parse_term();
		p->addChild(parse_term(), 1);
		token = getToken();
	}
	ungetToken();
	return t;
}

Parser::TreeNode *Parser::parse_term()
{
	TreeNode *t = parse_factor();
	Scanner::Token token = getToken();
	while (token.lexeme == "*" || token.lexeme == "/") {
		TreeNode *p = new TreeNode(OPERATION_K);
		p->setNodeKind(OPERATION_K);
		p->setToken(token);
		//p->child[0] = t;
		p->addChild(t, 0);
		t = p;
		//p->child[1] = parse_factor();
		p->addChild(parse_factor(), 1);
		token = getToken();
	}
	ungetToken();
	return t;
}

Parser::TreeNode *Parser::parse_factor()
{
	TreeNode *t = nullptr;
	Scanner::Token token = getToken();
	if (token.lexeme == "-") {
		t = new TreeNode(NEGATIVE_K);
		t->setNodeKind(NEGATIVE_K);
		t->setToken(token);
		//t->child[0] = parse_positive_factor();
		t->addChild(parse_positive_factor(), 0);
	}
	else {
		ungetToken();
		t = parse_positive_factor();
	}
	return t;
}

Parser::TreeNode *Parser::parse_positive_factor()
{
	TreeNode *t = nullptr;
	Scanner::Token token = getToken();
	if (token.lexeme == "~") {
		t = new TreeNode(BOOL_EXPRESSION_K);
		t->setToken(token);
		t->setNodeKind(BOOL_EXPRESSION_K);
		//t->child[0] = parse_not_factor();
		t->addChild(parse_not_factor(), 0);
	}
	else {
		ungetToken();
		t = parse_not_factor();
	}
	return t;
}

Parser::TreeNode *Parser::parse_not_factor()
{
	TreeNode *t = nullptr;
	Scanner::Token token = getToken();
	if (token.lexeme == "(") {
		t = parse_expression();
		token = getToken();
		if (token.lexeme != ")") {
			syntaxError(_strCurParserFileName, ")", token);
			return t;
		}
	}
	else if (token.kind == Scanner::INT) {
		t = new TreeNode(INT_CONST_K);
		t->setToken(token);
		t->setNodeKind(INT_CONST_K);
	}
	else if (token.kind == Scanner::CHAR) {
		t = new TreeNode(CHAR_CONST_K);
		t->setToken(token);
		t->setNodeKind(CHAR_CONST_K);
	}
	else if (token.kind == Scanner::STRING) {
		t = new TreeNode(STRING_CONST_K);
		t->setToken(token);
		t->setNodeKind(STRING_CONST_K);
	}
	else if (token.lexeme == "true" || token.lexeme == "false") {
		t = new TreeNode(BOOL_CONST_K);
		t->setToken(token);
		t->setNodeKind(BOOL_CONST_K);
	}
	else if (token.lexeme == "this") {
		t = new TreeNode(THIS_K);
		t->setToken(token);
		t->setNodeKind(THIS_K);
	}
	else if (token.lexeme == "null") {
		t = new TreeNode(NULL_K);
		t->setToken(token);
		t->setNodeKind(NULL_K);
	}
	else if (token.kind == Scanner::ID) {
		t = new TreeNode(VAR_K);
		t->setToken(token);
		t->setNodeKind(VAR_K);
		token = getToken();
		if (token.lexeme == "[") {
			TreeNode *p = parse_expression();
			//t->child[0] = p;
			t->addChild(p, 0);
			token = getToken();
			if (token.lexeme != "]") {
				syntaxError(_strCurParserFileName, "]", token);
				return t;
			}
			t->setNodeKind(ARRAY_K);
		}
		else if (token.lexeme == "(" || token.lexeme == ".") {
			ungetToken();
			ungetToken();
			t = parse_call_expression();
		}
		else
			ungetToken();
	}
	return t;
}

Parser::TreeNode *Parser::parse_call_expression()
{
	TreeNode *t = new TreeNode(CALL_EXPRESSION_K);
	t->setNodeKind(CALL_EXPRESSION_K);
	Scanner::Token token = getToken();
	Scanner::Token save = token;
	//t->child[0] = new TreeNode;
	auto node = new TreeNode;
	t->addChild(node, 0);
	token = getToken();
	if (token.lexeme == "(") {
		node->setNextNode(parse_expressions());
		token = getToken();
		if (token.lexeme != ")") {
			syntaxError(_strCurParserFileName, ")", token);
			return t;
		}
	}
	else if (token.lexeme == ".") {
		token = getToken();
		if (token.kind != Scanner::ID) {
			syntaxError(_strCurParserFileName, "identifier", token);
			return t;
		}
		save.lexeme = save.lexeme + "." + token.lexeme;
		token = getToken();
		if (token.lexeme != "(") {
			syntaxError(_strCurParserFileName, "(", token);
			return t;
		}
		node->setNextNode(parse_expressions());
		token = getToken();
		if (token.lexeme != ")") {
			syntaxError(_strCurParserFileName, ")", token);
			return t;
		}
	}
	t->setToken(save);
	return t;
}

void Parser::print()
{
	printSyntaxTree(_pSyntaxTree);
}

void Parser::printSyntaxTree(TreeNode *tree, int dep)
{
	while (tree != nullptr) {
		cout << "\n";
		for (int i = 0; i < dep; i++)
			cout << "  ";
		cout << "| ";
		switch (tree->getNodeKind()) {
		case CLASS_K:
			cout << "class" ;
			break;
		case CLASS_VAR_DEC_K:
			cout << "class_var_dec" ;
			break;
		case SUBROUTINE_DEC_K:
			cout << "subroutine_dec" ;
			break;
		case BASIC_TYPE_K:
			cout << "basic_type " << tree->getLexeme() ;
			break;
		case CLASS_TYPE_K:
			cout << "class_type " << tree->getLexeme();
			break;
		case PARAM_K:
			cout << "param" ;
			break;
		case VAR_DEC_K:
			cout << "var_dec" ;
			break;
		case ARRAY_K:
			cout << "array" ;
			break;
		case VAR_K:
			cout << "var" ;
			break;
		case IF_STATEMENT_K:
			cout << "if_statement" ;
			break;
		case WHILE_STATEMENT_K:
			cout << "while_statement" ;
			break;
		case RETURN_STATEMENT_K:
			cout << "return_statement" ;
			break;
		case CALL_STATEMENT_K:
			cout << "call_statement" ;
			break;
		case CALL_EXPRESSION_K:
			cout << "call_expression";
			break;
		case BOOL_EXPRESSION_K:
			cout << "bool_expression " << tree->getLexeme();
			break;
		case COMPARE_K:
			cout << "compare " << tree->getLexeme();
			break;
		case OPERATION_K:
			cout << "operation " << tree->getLexeme();
			break;
		case BOOL_K:
			cout << "bool" ;
			break;
		case ASSIGN_K:
			cout << "assign" ;
			break;
		case SUBROUTINE_BODY_K:
			cout << "subroutine_body" ;
			break;
		}
		/*printSyntaxTree(tree->child[0], dep + 2);
		printSyntaxTree(tree->child[1], dep + 2);
		printSyntaxTree(tree->child[2], dep + 2);
		printSyntaxTree(tree->child[3], dep + 2);
		printSyntaxTree(tree->child[4], dep + 2);*/

		for (int i = 0; i < TreeNode::Child_Num_Const; i++)  {
			printSyntaxTree(tree->getChildByIndex(i), dep + 2);
		}
		tree = tree->getNextNode();
	}
}

Parser::TreeNode *Parser::getSyntaxTree()
{
	return _pSyntaxTree;
}
