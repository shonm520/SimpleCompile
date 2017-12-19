#ifndef _ERROR_H
#define _ERROR_H

#include "Scanner.h"
class GramTreeNodeBase;

extern int errorNum;

bool hasError();
void syntaxError(string currentParserFilename, string expected, Scanner::Token token);

// �����ͺ�������һ��
void error1(string currentParserFilename);

// �����ض���
void error2(string currentClass, int row, string type, string name);

// �����ض���
void error3(string currentClass, int row, string type, string name);

// ����δ����
void error4(string currentClassName, int row, string type);

// ����δ����
void error5(string currentClassName, int row, string varName);

// ���Ͳ�ƥ��
void error6(string currentClassName, int row, string type);

// ����δ����
void error7(string currentClassName, string callerName, int row, string functionName);

// �������Ͳ�һ��
void error8(string currentClassName, int row, string functionName);

// �������ʹ���
void error9(string currentClassName, string callerName, int row, string functionName);

// �������ʹ���
void error10(string currentClassName, string callerName, int row, string functionName);

// ����ֵ����
void error11(string currentClassName, string type, int row);

// ����ֵ����
void error12(string currentClassName, int row);

// ����ֵ����
void error13(string currentClassName, int row);

// ����̫��
void error14(string currentClassName, string functionName, int row);

// ����̫��
void error15(string currentClassName, string functionName, int row);

void error16();

void error17();

void error18();

void error19();

void error20();

void error21(string currentClassName, string callerName, int row, string functionName);

void error22(GramTreeNodeBase* node, string msg);

#endif
