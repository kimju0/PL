/* front.c - a lexical analyzer system for simple arithmetic expressions */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <cstring>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>

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

//Lexical Analyzer
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
int n_ID, n_CONST, n_OP;
string target_equation;
int n_paren;
string LeftTerm;
int res_RightTerm;

void addChar();
void getChar();
void getNonBlank();
int lex();
void parser();

int find_ident(string);


int main(int argc, char** argv) {
    string input=string(argv[1]);
    int index;
    
    if ((index=input.find(string("-v")))!= string::npos ) {
        input=input.substr(0, index);
        compile_option_v=true;
    }
    
    if ((in_fp = fopen(input.c_str(), "r")) == NULL) {
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
        n_OP++;
        break;
    case '-':
        addChar();
        nextToken = SUBOP;
        n_OP++;
        break;
    case '*':
        addChar();
        nextToken = MULTOP;
        n_OP++;
        break;
    case '/':
        addChar();
        nextToken = DIVOP;
        n_OP++;
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
        n_ID++;
        addChar();
        getChar();
        while (charClass == LETTER || charClass == DIGIT) {
            addChar();
            getChar();
        }
        nextToken = IDENTIFIER;

        break;

        /* Parse integer literals */
    case DIGIT:
        n_CONST++;
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
    target_equation += string(lexeme);
    target_equation += " ";
    if (compile_option_v)
        printf("Next Token : %s\nNext lexeme : %s\n\n", tokenName[nextToken+1].c_str(), lexeme);
    return nextToken;
}

int find_ident(string target) {
    for (auto i = 0; i < ident.size(); i++) {
        if (ident[i].first == string(target)) {
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
bool operate();//선언되지 않은 식별자 유무 반환

enum ERROR_CODE {
    no_declaration,
};

void error(int debug) {
	printf("Error:%d\n",debug);
    error_flag = true;
}

void program() {
    statements();
}
void statements() {
    statement();
    if (!compile_option_v)
        printf("%s", input.c_str());
    if (!compile_option_v)
        printf("ID: %d; CONST: %d; OP: %d;\n", n_ID, n_CONST, n_OP);
    n_ID = 0, n_CONST = 0, n_OP = 0;
    if (!compile_option_v)
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
        LeftTerm = string(lexeme);
		lex();
        if (nextToken == ASSIGNMENT) {
            target_equation = string("");
            res_RightTerm = 0;
			lex();
			expression();
            //printf("\n\n%s\n\n", target_equation.c_str());//debug
            if(operate())
                ident.push_back(make_pair(LeftTerm, res_RightTerm));
            else {
				error(no_declaration);
			}
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

string actual_operate(string target) {//단순 연산
    //input은 괄호 없는 수식
    //target = "1 + 2 + 1234 / 3 * 3";//debug
    stringstream ss(target);
    vector <string> tokens;
    int num_mul = 0;
    for (string i; ss >> i;) {
		tokens.push_back(i);
        if (i == "*" || i == "/") {
            num_mul++;
        }
	}
    for (int i = 0; i < num_mul; i++) {
        for(int j=0;j<tokens.size();j++)
            if (tokens[j] == "*" || tokens[j] == "/") {
                int a,b;
                if(isdigit(tokens[j - 1].front()))
                    a= stoi(tokens[j - 1]);
                else
                    a=ident[find_ident(tokens[j - 1])].second;
                
                if (isdigit(tokens[j + 1].front()))
                    b = stoi(tokens[j + 1]);
                else
                    b = ident[find_ident(tokens[j + 1])].second;
				
                int res;
				if (tokens[j] == "*")
					res = a * b;
				else
					res = a / b;
				tokens[j - 1] = to_string(res);
				tokens.erase(tokens.begin() + j);
				tokens.erase(tokens.begin() + j);
				break;
			}
    }
    while (tokens.size() >= 3) {
        for (int j = 0; j < tokens.size(); j++)
            if (tokens[j] == "+" || tokens[j] == "-") {
                int a, b;
                if (isdigit(tokens[j - 1].front()))
                    a = stoi(tokens[j - 1]);
                else
                    a = ident[find_ident(tokens[j - 1])].second;

                if (isdigit(tokens[j + 1].front()))
                    b = stoi(tokens[j + 1]);
                else
                    b = ident[find_ident(tokens[j + 1])].second;
                int res;
                if (tokens[j] == "+")
                    res = a + b;
                else
                    res = a - b;
                tokens[j - 1] = to_string(res);
                tokens.erase(tokens.begin() + j);
                tokens.erase(tokens.begin() + j);
                break;
            }
    }
    return tokens.front();
}
bool operate() {//괄호 처리
    if(target_equation.find(";")!=string::npos)
        target_equation=target_equation.substr(0, target_equation.find(";") );
    for (int i = 0; i < n_paren; i++) {//괄호 개수만큼 반복
        int start_paren=0;
        for (int j = 0; j < target_equation.size(); i++) {
            if (target_equation[j] == '(')
                start_paren = j;
            else if (target_equation[j] == ')') {
                string replace = actual_operate(target_equation.substr(start_paren + 1, j - start_paren - 1));
                target_equation.replace(start_paren, j - start_paren + 1, replace);
            }
        }
    }
    res_RightTerm= stoi(actual_operate(target_equation));
	return true;
}

void parser() {
    getChar();
    lex();
	program();
    if(!compile_option_v)
        printf("Result ==> ");
    if (!compile_option_v)
        for (int i = 0; i < ident.size(); i++) {
            printf("%s: %d; ",ident[i].first.c_str(), ident[i].second);
        }
    if (!compile_option_v)
        printf("\n");
    if (nextToken == TC_END) {
		printf("Parsing Complete\n");
	}
    else {
		error(3);
	}
}