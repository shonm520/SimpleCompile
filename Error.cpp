#include <iostream>
#include "Error.h"
#include "GramTreeNode.h"

using namespace std;

int errorNum;

bool hasError()
{
    return errorNum;
}

void syntaxError(string currentParserFilename, string expected, Scanner::Token token)
{
    errorNum++;
    cerr << "Error in class " << currentParserFilename << " in line " << token.row
        << ": expect " << "\'" << expected << "\'" << ", but got " << "\'" << token.lexeme << "\'" << "\n";
}

// �����ͺ�������һ��
void error1(string currentParserFilename)
{
    errorNum++;
    cerr << "Error in file " << currentParserFilename << ".jack: " << "classname should be same as filename" << endl;
}

// �����ض���
void error2(string currentClass, int row, string type, string name)
{
    errorNum++;
    cerr << "Error in class " << currentClass << " in line " << row
        << ": redeclaration of '" << type << " " << name << "'" << endl;
}

// �����ض���
void error3(string currentClass, int row, string type, string name)
{
    errorNum++;
    cerr << "Error in class " << currentClass << " in line " << row
        << ": redeclaration of '" << type << " " << name << "()" << endl;
}

// ����δ����
void error4(string currentClassName, int row, string type)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": '" << type << "' not declaraed" << endl;
}

// ����δ����
void error5(string currentClassName, int row, string varName)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": '" << varName << "' does not declared in this scope" << endl;
}

void error6(string currentClassName, int row, string type)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": " << type << " does not an Array type" << endl;
}

void error7(string currentClassName, string callerName, int row, string functionName)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": class " << callerName << " haven't a member function '" << functionName << "()'" << endl;
}

void error8(string currentClassName, int row, string functionName)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": subroutine " << functionName << " called as a method from within a function" << endl;
}

void error9(string currentClassName, string callerName, int row, string functionName)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": '" << functionName << "' is not a function in class " << callerName << endl;
}

// �������ʹ���
void error10(string currentClassName, string callerName, int row, string functionName)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": '" << functionName << "' is not a method in class " << callerName << endl;
}

// ����ֵ����
void error11(string currentClassName, string type, int row)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": return-statement with no value, in function returning '" << type << "'" << endl;
}

// ����ֵ����
void error12(string currentClassName, int row)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": return-statement with a value, in function returning void" << endl;
}

// ����ֵ����
void error13(string currentClassName, int row)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": The return type of a constructor must be of the class type" << endl;
}

// ����̫��
void error14(string currentClassName, string functionName, int row)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": too few arguments to function " << functionName << "()" << endl;
}

// ����̫��
void error15(string currentClassName, string functionName, int row)
{
    errorNum++;
    cerr << "Error in class " << currentClassName << " in line " << row
        << ": too many arguments to function " << functionName << endl;
}

void error16()
{
    errorNum++;
    cerr << "Error: Main class not exsist" << endl;
}

void error17()
{
    errorNum++;
    cerr << "Error in class Main: main function does not exsit!" << endl;
}

void error18()
{
    errorNum++;
    cerr << "Error in class Main: the kind of subroutine main must be a function" << endl;
}

void error19()
{
    errorNum++;
    cerr << "Error in class Main: the type of subroutine main must be a void" << endl;
}

void error20()
{
    errorNum++;
    cerr << "Error in class Main: the argument size of subroutine main must be null" << endl;
}

void error21(string currentClassName, string callerName, int row, string functionName)
{
	errorNum++;
	cerr << "Error in class " << currentClassName << " in line " << row
		<<", function(or method) "<< "\'" << functionName << "\' " << "is not defined" << endl;
}

void error22(GramTreeNodeBase* node, string msg)
{
	errorNum++;
	cerr << "Error in class " << node->getClassName() << " in line " << node->getRow()
		<< ", in function(or method) " << "\'" 
		<<  node->getCurSubroutineBodyNode()->getParentNode()->getName() 
		<< "\', " << "�ڵ� " << node->getLexeme() << "," << msg << endl;
}
