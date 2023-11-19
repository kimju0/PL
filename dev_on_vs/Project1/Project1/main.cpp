/* front.c - a lexical analyzer system for simple arithmetic expressions */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <cstring>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;

enum CharClass
{
    CC_END = -1,
	LETTER,
	DIGIT,
	UNKNOWN
};
enum TokenCodes
{
    TC_END = -1,
    CONST,
    IDENTIFIER,
    ASSIGNMENT,
    SEMICOLON,
    ADDOP,
    SUBOP,
    MULTOP,
    DIVOP,
    LEFTPAREN,
    RIGHTPAREN,
};
string tokenName[] = { "EOF", "CONST", "IDENTIFIER", "ASSIGNMENT", "SEMICOLON", "ADDOP", "SUBOP", "MULTOP", "DIVOP", "LEFTPAREN", "RIGHTPAREN" };

CharClass charClass;
char lexeme[100];
char nextChar;
int lexLen;
int token;
TokenCodes nextToken;
FILE* in_fp, * fopen();

bool compile_option_v = false;

//Parser
string input;//main함수 안에도 있음
bool error_flag = false;
vector<pair<string, int>> ident;


/* Function declarations */
void addChar();
void getChar();
void getNonBlank();
int lex();

void parser();

int find_ident(string);


int main(int argc, char** argv) {
    string input=argv[1];
    int index;
    
    if (index=input.find("-v")!= string::npos ) {
        input.substr(0, index);
        compile_option_v=true;
    }
    
    if ((in_fp = fopen(argv[1], "r")) == NULL) {
        printf("ERROR_fopen");
    }
    else {
        parser();
        /*
        getChar();
        do {
            lex();
        } while (nextToken != EOF);
        */
    }
    
    return 0;
}

//연산자 및 괄호를 lookup하여 token을 반환
int lookup(char ch) {
    switch (ch) {
    //예외처리 ':='가 되야함
    case '=':
		addChar();
		nextToken = ASSIGNMENT;
		break;
    case ';':
		addChar();
		nextToken = SEMICOLON;
		break;
    case '(':
        addChar();
        nextToken = LEFTPAREN;
        break;
    case ')':
        addChar();
        nextToken = RIGHTPAREN;
        break;
    case '+':
        addChar();
        nextToken = ADDOP;
        break;
    case '-':
        addChar();
        nextToken = SUBOP;
        break;
    case '*':
        addChar();
        nextToken = MULTOP;
        break;
    case '/':
        addChar();
        nextToken = DIVOP;
        break;

    default:
        addChar();
        nextToken = TC_END;
        break;
    }
    return nextToken;
}

//lexeme에 nextChar 추가
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = 0;
    }
}

//문자 하나 nextChar에 가져와서 charClass에 저장
void getChar() {
    if ((nextChar = getc(in_fp)) != EOF) {
        input.push_back(nextChar);
        //printf("%s\n", input.c_str());
        if (isalpha(nextChar))
            charClass = LETTER;
        else if (isdigit(nextChar))
            charClass = DIGIT;
        else charClass = UNKNOWN;
    }
    else
        charClass = CC_END;
}

//nextChar가 공백이 아닐 때까지 getChar() 호출
void getNonBlank() {
    while (isspace(nextChar)) {
        getChar();
    }
}

//lexeme을 읽어서 token을 반환
int lex() {
    lexLen = 0;
    getNonBlank();
    switch (charClass) {

        /* Parse identifiers */
    case LETTER:
        addChar();
        getChar();
        while (charClass == LETTER || charClass == DIGIT) {
            addChar();
            getChar();
        }
        nextToken = IDENTIFIER;

        if (find_ident(string(lexeme)) != -1) {
			ident.push_back(make_pair(string(lexeme), 0));
        }
        break;

        /* Parse integer literals */
    case DIGIT:
        addChar();
        getChar();
        while (charClass == DIGIT) {
            addChar();
            getChar();
        }
        nextToken = CONST;
        break;

        /* Parentheses and operators */
    case UNKNOWN:
        if (nextChar == ':') {
            addChar();
            getChar();
        }
        lookup(nextChar);
        getChar();
        break;

        /* EOF */
    case EOF:
        nextToken = TC_END;
        lexeme[0] = 'E';
        lexeme[1] = 'O';
        lexeme[2] = 'F';
        lexeme[3] = 0;
        break;
    }
    if (compile_option_v)
        printf("Next Token : %s\nNext lexeme : %s\n\n", tokenName[nextToken+1].c_str(), lexeme);
    return nextToken;
}

int find_ident(string target) {
    nextToken = IDENTIFIER;
    for (auto i = 0; i < ident.size(); i++) {
        if (ident[i].first == string(lexeme)) {
            return i;
        }
    }
    return -1;
}

//PARSER
void program();
void statements();
void statement();
void expression();
void term_tail();
void term();
void factor_tail();
void factor();
void error(int debug);


void error(int debug) {
	printf("Error:%d\n",debug);
    error_flag = true;
}

void program() {
    statements();
}

void statements() {
    statement();
    printf("%s", input.c_str());
    printf("ID: %d; CONST: %d; OP: %d;\n",0,0,0);
    printf(
        error_flag ? "(Error)\n" : "(OK)\n"
    );
    if (nextToken == SEMICOLON) {
        input = "";
        error_flag = false;
		lex();
		statements();
	}
}
void statement() {
    if (nextToken == IDENTIFIER) {
		lex();
        if (nextToken == ASSIGNMENT) {
			lex();
			expression();
		}
        else {
			error(1);
		}
	}
    else {
		error(1);
	}
}
void expression() {
	term();
	term_tail();
}
void term_tail() {
    if (nextToken == ADDOP || nextToken == SUBOP) {
		lex();
		term();
		term_tail();
	}
    //lambda인 경우 PASS 다시 생각해보기
}
void term() {
	factor();
	factor_tail();
}
void factor_tail() {
    if (nextToken == MULTOP || nextToken == DIVOP) {
		lex();
		factor();
		factor_tail();
	}
	//lambda인 경우 PASS 다시 생각해보기
}
void factor() {
    if (nextToken == IDENTIFIER || nextToken == CONST) {
		lex();
	}
    else if (nextToken == LEFTPAREN) {
		lex();
		expression();
        if (nextToken == RIGHTPAREN) {
			lex();
		}
        else {
			error(2);
		}
	}
    else {
		error(2);
	}
}
void parser() {
    getChar();
    lex();
	program();
    printf("Result ==> ");
    for (int i = 0; i < ident.size(); i++) {
        printf("%s = %d ",ident[i].first,ident[i].second);
    }
    printf("\n");
    if (nextToken == TC_END) {
		printf("Parsing Complete\n");
	}
    else {
		error(3);
	}
}