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

bool compile_option_v = false;

//data type
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
string tokenName[] = { 
    "EOF", "CONST", "IDENTIFIER", "ASSIGNMENT", "SEMICOLON", "ADDOP", "SUBOP", "MULTOP", "DIVOP", "LEFTPAREN", "RIGHTPAREN" 
};
enum ERROR_CODE {
    NO_ERROR,
    no_declaration,
    not_enough_elements,
    wrong_parenthesis
};
string errorstring[] = {
    string("\"NO error\""),
    string("\"정의되지 않은 변수가 참조됨\""),
    string("\"식의 요소가 부족함\""),
    string("\"올바르지 않은 괄호쌍\"")
};

//Lexical Analyzer-Global Variables
CharClass charClass;
char lexeme[100];
char nextChar;
int lexLen;
int token;
TokenCodes nextToken;
FILE* in_fp, * fopen();

//Lexical Analyzer-Function Declaration
void addChar();
void getChar();
void getNonBlank();
int lex();
void parser();
int find_ident(string);
void push_ident(string, int);

//Parser-Global Variables
string input;
bool global_error_flag = false;
int error_flag = 0;
vector<pair<string, int>> ident;
int n_ID, n_CONST, n_OP;
string target_equation;
int n_close_paren, n_open_paren;
string LeftTerm;
int res_RightTerm;

//Parser-Function Declaration
void program();
void statements();
void statement();
void expression();
void term_tail();
void term();
void factor_tail();
void factor();
void error(int debug);
int operate();//선언되지 않은 식별자 유무 반환

//DEFINITION OF FUNCTION
//*********************************************************************************
//*********************************************************************************

int main(int argc, char** argv) {
    string input=string(argv[1]);
    int index;

    if(argc==3&&string(argv[2])=="-v")compile_option_v = true;
    
    if ((in_fp = fopen(input.c_str(), "r")) == NULL) {
        printf("ERROR_fopen");
    }
    else {
        parser();
    }
    
    return 0;
}

//연산자 및 괄호를 lookup하여 token을 반환
int lookup(char ch) {
    switch (ch) {
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
        n_open_paren++;
        break;
    case ')':
        addChar();
        nextToken = RIGHTPAREN;
        n_close_paren++;
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
    while (nextChar <= 32&&nextChar>=0) {
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

void push_ident(string target, int value) {
    int ind=find_ident(target);
	if(ind==-1)ident.push_back(make_pair(target, value));
	else ident[ind].second = value;
}

void error(int debug) {
	//printf("Error:%d\n",debug);
    global_error_flag = true;
    error_flag = debug;
}

void program() {
    statements();
}
void statements() {
    error_flag = false;
    statement();
    if (input.find("\n") != string::npos)
        input=input.substr(0, input.find("\n"));
    if (!compile_option_v)
        printf("%s\n", input.c_str());
    if (!compile_option_v)
        printf("ID: %d; CONST: %d; OP: %d;\n", n_ID, n_CONST, n_OP);
    n_ID = 0, n_CONST = 0, n_OP = 0;
    if (!compile_option_v) {
        if (error_flag)printf("(Error) %s\n", errorstring[error_flag].c_str());
        else printf("(OK)\n");
    }
    if (nextToken == SEMICOLON) {
        input = "";
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
            n_close_paren = 0, n_open_paren = 0;
			lex();
			expression();
            //printf("\n\n%s\n\n", target_equation.c_str());//debug
            int code;
            if ((code = operate()) == 0)
                push_ident(LeftTerm, res_RightTerm);

            else {
				error(code);
				push_ident(LeftTerm, 0);
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
                if (isdigit(tokens[j - 1].front()))
                    a = stoi(tokens[j - 1]);
                else
                    if (find_ident(tokens[j - 1]) == -1) {
                        error(no_declaration);
                        push_ident(tokens[j - 1], 0);
                        a = 0;
                    }
                    else
                        a=ident[find_ident(tokens[j - 1])].second;
                
                if (isdigit(tokens[j + 1].front()))
                    b = stoi(tokens[j + 1]);
                else
                    if (find_ident(tokens[j + 1]) == -1) {
                        error(no_declaration);
                        push_ident(tokens[j + 1], 0);
                        b = 0;
                    }
                    else
                        b = ident[find_ident(tokens[j + 1])].second;
				
                int res;
				if (tokens[j] == "*")
					res = a * b;
				else
					res = (b==0)?0:a/b;
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
                    if (find_ident(tokens[j - 1]) == -1) {
                        error(no_declaration);
                        push_ident(tokens[j - 1], 0);
                        a = 0;
                    }
                    else
                        a = ident[find_ident(tokens[j - 1])].second;

                if (isdigit(tokens[j + 1].front()))
                    b = stoi(tokens[j + 1]);
                else
                    if (find_ident(tokens[j + 1]) == -1) {
                        push_ident(tokens[j + 1], 0);
                        error(no_declaration);
                        b = 0;
                    }
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
int operate() {//괄호 처리
    if (n_OP + 1 != n_ID-1+n_CONST)return 2;
    if(n_open_paren!=n_close_paren)return 3;
    if(target_equation.find(";")!=string::npos)
        target_equation=target_equation.substr(0, target_equation.find(";") );
    for (int i = 0; i < n_close_paren; i++) {//괄호 개수만큼 반복
        int start_paren=-1;
        for (int j = 0; j < target_equation.size(); j++) {
            if (target_equation[j] == '(')
                start_paren = j;
            else if (target_equation[j] == ')') {
                if(start_paren==-1)return 3;//닫는 괄호가 먼저 나오는 case
                string replace = actual_operate(target_equation.substr(start_paren + 1, j - start_paren - 1));
                target_equation.replace(start_paren, j - start_paren + 1, replace);
            }
        }
    }
    res_RightTerm= stoi(actual_operate(target_equation));
	return 0;
}

void parser() {
    getChar();
    lex();
	program();
    if(!compile_option_v)
        printf("Result ==> ");
    if (!compile_option_v)
        if (global_error_flag) {
            for (int i = 0; i < ident.size(); i++) {
                printf("%s: Unknown; ", ident[i].first.c_str());
            }
        }
        else {
            for (int i = 0; i < ident.size(); i++) {
                printf("%s: %d; ", ident[i].first.c_str(), ident[i].second);
            }
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