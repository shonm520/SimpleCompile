#ifndef _Scanner_H_
#define _Scanner_H_

#include <string>
#include <fstream>
#include <deque>
#include <set>

using namespace std;

class Scanner
{
private:
    enum State	// ת��ͼ�е�״̬
    {
		STATE_UNREADY = -1,
        STATE_START,		// ��ʼ״̬
        STATE_ID,			// ��ʶ��״̬
        STATE_INT,			// ������״̬
        STATE_CHAR,			// �ַ�״̬		
        STATE_CHAR_A,
        STATE_CHAR_B,
        STATE_CHAR_C,
        STATE_FLOAT,		// ������״̬
        STATE_D_FLOAT,		// �ӽ���С����ĸ�����״̬
        STATE_E_FLOAT,		// �ӽ���ѧ�������ĸ�����״̬
        STATE_STRING,		// �ַ���״̬
        STATE_S_STRING,		// ����ת���ַ����ַ���
        STATE_SYMBOL, 
        STATE_COMMENT,   	// ע��״̬
        STATE_P_COMMENT,	// ��Ҫ����ע��״̬
        STATE_DONE,			// ����״̬
        STATE_ERROR			// ����״̬
    };

public:
    set<string> m_vtkeyWords;
    set<string> m_vtStrSymbols;
    enum TokenType
    {      
        KEY_WORD,
        ID,				// ��ʶ��
        INT,			// ��������
        BOOL,			// ��������
        CHAR,			// �ַ�
        STRING,			// �ַ���
        SYMBOL,         // �Ϸ��ķ���
        NONE,		    // ������
        ERROR,		    // ����
        ENDOFFILE	    // �ļ�����
    };
    struct Token
    {
        TokenType kind;				// Token������
        string lexeme;				// Token��ֵ
        unsigned row;	   	        // ��ǰ��
    };
    void initKeyWords();
    void initSymbols();
private:
    string _strLineBuffer;					// ������, ����Դ�����е�һ������
    unsigned _nBufferPos;					// �����е�ָ��
    unsigned _row;						// ���浱ǰ��������Դ�����е��к�
    ifstream _fIn;						// Դ�����ļ�������������
    char nextChar();					// ���ػ������е���һ���ַ�
    void rollBack();					// �ع�������
    TokenType searchReserved(string &s);	// ���ҹؼ���

	State procStartState(char ch, Token& token);
	bool procIntState(char ch, Token& token);
	bool procIdentityState(char ch, Token& token);
	void procStringState(char ch, Token& token, Scanner::State& state);
	void procCharState(char ch, Token& token, Scanner::State& state);
	void procSymbolState(char ch, Token& token, Scanner::State& state);
	void procCommentState(char ch, Token& token, Scanner::State& state);
public:
    Scanner();
    int openFile(string filename);
    void closeFile();
    Token nextToken();					// ������һ��Token
    void resetRow();
};

#endif
