/* front.c - a lexical analyzer system for simple arithmetic expressions */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>

/* Global declarations */
/* Variables */
int charClass;
char lexeme[100];
char nextChar;
int lexLen;
int token;
int nextToken;
FILE* in_fp, * fopen();

/* Function declarations */
void addChar();
void getChar();
void getNonBlank();
int lex();

/* Character classes */
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99

/* Token codes */
#define INT_LIT 10
#define IDENT 11
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 25
#define RIGHT_PAREN 26

/******************************************************/
/* main driver */

int main(int argc, char** argv) {
    printf("Starting ..... \n");

    /* Open the input data file and process its contents */
    if ((in_fp = fopen(argv[1], "r")) == NULL) {
        printf("ERROR");
    }
    else {
        getChar();
        do {
            lex();
        } while (nextToken != EOF);
    }
    return 0;
}


/*****************************************************/
/* lookup - a function to lookup operators and parentheses and return the token */
int lookup(char ch) {
    switch (ch) {
    case '(':
        addChar();
        nextToken = LEFT_PAREN;
        break;
    case ')':
        addChar();
        nextToken = RIGHT_PAREN;
        break;
    case '+':
        addChar();
        nextToken = ADD_OP;
        break;
    case '-':
        addChar();
        nextToken = SUB_OP;
        break;
    case '*':
        addChar();
        nextToken = MULT_OP;
        break;
    case '/':
        addChar();
        nextToken = DIV_OP;
        break;

    default:
        addChar();
        nextToken = EOF;
        break;
    }
    return nextToken;
}

/*****************************************************/
/* addChar - a function to add nextChar to lexeme */
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = 0;
    }
}

/*****************************************************/
/* getChar - a function to get the next character of
input and determine its character class */
void getChar() {
    if ((nextChar = getc(in_fp)) != EOF) {
        if (isalpha(nextChar))
            charClass = LETTER;
        else if (isdigit(nextChar))
            charClass = DIGIT;
        else charClass = UNKNOWN;
    }
    else
        charClass = EOF;
}


/*****************************************************/
/* getNonBlank - a function to call getChar until it
returns a non-whitespace character */
void getNonBlank() {
    while (isspace(nextChar)) {
        getChar();
    }
}

/*****************************************************/
/* lex - a simple lexical analyzer for arithmetic expressions */
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
        nextToken = IDENT;
        break;

        /* Parse integer literals */
    case DIGIT:
        addChar();
        getChar();
        while (charClass == DIGIT) {
            addChar();
            getChar();
        }
        nextToken = INT_LIT;
        break;

        /* Parentheses and operators */
    case UNKNOWN:
        lookup(nextChar);
        getChar();
        break;

        /* EOF */
    case EOF:
        nextToken = EOF;
        lexeme[0] = 'E';
        lexeme[1] = 'O';
        lexeme[2] = 'F';
        lexeme[3] = 0;
        break;
    } /* End of switch */
    printf("Next Token is : %d, Next lexeme is %s\n", nextToken, lexeme);
    return nextToken;
} /* End of function lex */




//----------------------------------------------
//parser
#define SEMICOLON 27//lexical analyzer에 추가

void program();
void statements();
void statement();
void expression();
void term_tail();
void term();
void factor_tail();
void factor();
void error();


void error() {
	printf("Error");
    //무슨 에러인지 출력
}
void program() {
    statements();
}
void statements() {
    statement();
    if (nextToken == SEMICOLON) {
		lex();
		statements();
	}
}
void statement() {
    if (nextToken == IDENT) {
		lex();
        if (nextToken == ASSIGN_OP) {
			lex();
			expression();
		}
        else {
			error();
		}
	}
    else {
		error();
	}
}
void expression() {
	term();
	term_tail();
}
void term_tail() {
    if (nextToken == ADD_OP || nextToken == SUB_OP) {
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
    if (nextToken == MULT_OP || nextToken == DIV_OP) {
		lex();
		factor();
		factor_tail();
	}
	//lambda인 경우 PASS 다시 생각해보기
}
void factor() {
    if (nextToken == IDENT || nextToken == INT_LIT) {
		lex();
	}
    else if (nextToken == LEFT_PAREN) {
		lex();
		expression();
        if (nextToken == RIGHT_PAREN) {
			lex();
		}
        else {
			error();
		}
	}
    else {
		error();
	}
}


////parser
///*
//GRAMMAR
//<expr> ® <term> {(+| -) < term > }
//<term> ® <factor> {(*| /) < factor > }
//<factor> ® id | int_constant | (<expr>)
//*/
//
///* Function expr
//Parses strings in the language
//generated by the rule:
//<expr> → <term> {(+ | -) <term>}
//*/
//void expr() {
//    /* Parse the first term */
//    term();
//    /* As long as the next token is + or -, call
//    lex to get the next token and parse the
//    next term */
//    while (nextToken == ADD_OP ||
//        nextToken == SUB_OP) {
//        lex();
//        term();
//    }
//} /* End of function expr */
//
///* Function term
//Parses strings in the language
//generated by the rule:
//<term> -> <factor> {(* | /) <factor>)
//*/
//void term() {
//    /* Parse the first factor */
//    factor();
//    /* As long as the next token is * or /,
//    next token and parse the next factor */
//    while (nextToken == MULT_OP || nextToken == DIV_OP) {
//        lex();
//        factor();
//    }
//} /* End of function term */
//
///* Function factor
//Parses strings in the language
//generated by the rule:
//<factor> -> id | (<expr>) 
//*/
//void factor() {
//    /* Determine which RHS */
//    if (nextToken == ID_CODE || nextToken == INT_CODE)
//    /* For the RHS id, just call lex */
//        lex();
//    /* If the RHS is (<expr>) ? call lex to pass over the left parenthesis,
//    call expr, and check for the right parenthesis */
//    else if (nextToken == LP_CODE) {
//        lex();
//        expr();
//        if (nextToken == RP_CODE)
//            lex();
//        else
//            error();
//    } /* End of else if (nextToken == ... */
//    else error(); /* Neither RHS matches */
//}