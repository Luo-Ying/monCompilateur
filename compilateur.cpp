//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"


#include <string>
#include <iostream>
#include <cstdlib>
#include <set>
#include <FlexLexer.h>
#include "tokeniser.h"
#include <cstring>

using namespace std;

enum OPREL {EQU, DIFF, INF, SUP, INFE, SUPE, WTFR};			//enum <类型名> {<枚举常量表>};
enum OPADD {ADD, SUB, OR, WTFA};
enum OPMUL {MUL, DIV, MOD, AND ,WTFM};
enum TYPES {UNSIGNED_INT, BOOLEAN};

TOKEN current;				// Current token


FlexLexer* lexer = new yyFlexLexer; // This is the flex tokeniser
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

	
set<string> DeclaredVariables;	// Store declared variables and their types
unsigned long TagNumber=0;

bool IsDeclared(const char *id){
	return DeclaredVariables.find(id)!=DeclaredVariables.end();		//.find()返回查找元素的位置, .end()返回最后一个元素的下一个位置
}


void Error(string s){
	cerr << "Ligne n°"<<lexer->lineno()<<", lu : '"<<lexer->YYText()<<"'("<<current<<"), mais ";
	cerr<< s << endl;
	exit(-1);
}

//  -- retourner le type de l'identificateur. (pour l'instant : UNSIGNED_INT) --
enum TYPES Identifier(void){
	enum TYPES type;
	cout << "\tpush "<<lexer->YYText()<<endl;
	current=(TOKEN) lexer->yylex();
	if( current!=TYPE ){
		Error("Type de variable inconnu");
	}
	// type = lexer->YYText();
	current = (TOKEN) lexer->yylex();
	return UNSIGNED_INT;
}

// -- retourner le type UNSIGNED_INT (prévoir qu'il y aura plusieurs possibilités dans le futur) --
enum TYPES Number(void){
	cout <<"\tpush $"<<atoi(lexer->YYText())<<endl;
	current=(TOKEN) lexer->yylex();
	return UNSIGNED_INT;
}

enum TYPES Expression(void);			// Called by Term() and calls Term()
void Statement(void);			// Called by les statements;


// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Letter {"," Letter} "]"
// StatementPart := Statement {";" Statement} "."
// (modifed) Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement
// (ajout) IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
// (ajout) WhileStatement := "WHILE" Expression DO Statement
// (ajout) ForStatement := "FOR" AssignementStatement "To" Expression "DO" Statement
// (ajout) BlockStatement := "BEGIN" Statement { ";" Statement } "END"
// AssignementStatement := Letter "=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor
// Number := Digit{Digit}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"


//  -- renvoie le type de l'analyse qu'elle a appelée, i.e. expression ou number ou encore identifier. --
enum TYPES Factor(void){
	enum TYPES type;
	// if(current==RPARENT){
	// 	current=(TOKEN) lexer->yylex();
	// 	Expression();
	// 	if(current!=LPARENT)
	// 		Error("')' était attendu");		// ")" expected
	// 	else
	// 		current=(TOKEN) lexer->yylex();
	// }
	// else 
	// 	if (current==NUMBER)
	// 		Number();
	//      	else
	// 			if(current==ID)
	// 				Identifier();
	// 			else
	// 				Error("'(' ou chiffre ou lettre attendue");
	switch( current ){
		case RPARENT:
			current = (TOKEN) lexer->yylex();
			type = Expression();
			if( current != LPARENT ){
				Error("')' était attendu");		// ")" expected
			}
			else{
				current = (TOKEN) lexer->yylex();
			}
			break;
		case NUMBER:
			type = Number();
			break;
		case ID:
			type = Identifier();
			break;
		default:
			Error("'(' ou chiffre ou lettre attendue");
	}
	return type;
}

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
OPMUL MultiplicativeOperator(void){
	OPMUL opmul;
	// strcmp() comparer les chaines de string par caractere, et renvoyer la difference des chaines, 0 sinon
	if(strcmp(lexer->YYText(),"*")==0)		// si strcmp() renvoie 0 alors c'est "*", sinon non
		opmul=MUL;
	else if(strcmp(lexer->YYText(),"/")==0)
		opmul=DIV;
	else if(strcmp(lexer->YYText(),"%")==0)
		opmul=MOD;
	else if(strcmp(lexer->YYText(),"&&")==0)
		opmul=AND;
	else opmul=WTFM;
	current=(TOKEN) lexer->yylex();
	return opmul;
}

// Term := Factor {MultiplicativeOperator Factor}
// récupère le type de Factor, vérifie que tous les appels suivants à Factor 
// renvoient le même type (ou un type compatible dans le futur). Sinon, Term 
// doit générer une erreur. Term renvoie le type retourné par tous les appels à Factor.
enum TYPES Term(void){
	TYPES type1, type2;
	OPMUL mulop;
	type1 = Factor();
	while(current==MULOP){
		mulop = MultiplicativeOperator();		// Save operator in local variable
		type2 = Factor();
		if( type1 != type2 ){
			Error("types incompatibles dans l'expression");
		}
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		switch(mulop){
			case AND:
				cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
				cout << "\tpush %rax\t# AND"<<endl;	// store result
				break;
			case MUL:
				cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
				cout << "\tpush %rax\t# MUL"<<endl;	// store result
				break;
			case DIV:
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// quotient goes to %rax
				cout << "\tpush %rax\t# DIV"<<endl;		// store result
				break;
			case MOD:
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// remainder goes to %rdx
				cout << "\tpush %rdx\t# MOD"<<endl;		// store result
				break;
			default:
				Error("opérateur multiplicatif attendu");
		}
	}
	return type1;
}

// AdditiveOperator := "+" | "-" | "||"
OPADD AdditiveOperator(void){
	OPADD opadd;
	if(strcmp(lexer->YYText(),"+")==0)
		opadd=ADD;
	else if(strcmp(lexer->YYText(),"-")==0)
		opadd=SUB;
	else if(strcmp(lexer->YYText(),"||")==0)
		opadd=OR;
	else opadd=WTFA;
	current=(TOKEN) lexer->yylex();
	return opadd;
}

// SimpleExpression := Term {AdditiveOperator Term}
// récupère le type de Term, vérifie que tous les appels suivants à Term renvoient le même type 
// (ou un type compatible dans le futur). Sinon, SimpleExpression doit générer une erreur. 
// SimpleExpression renvoie le type retourné par tous les appels à Terms.
enum TYPES SimpleExpression(void){
	enum TYPES type1, type2;
	OPADD adop;
	type1 = Term();
	while(current==ADDOP){
		adop=AdditiveOperator();		// Save operator in local variable
		type2 = Term();
		if( type2 != type1 ){
			Error("types incompatibles dans l'expression");
		}
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		switch(adop){
			case OR:
				cout << "\taddq	%rbx, %rax\t# OR"<<endl;// operand1 OR operand2
				break;			
			case ADD:
				cout << "\taddq	%rbx, %rax\t# ADD"<<endl;	// add both operands
				break;			
			case SUB:	
				cout << "\tsubq	%rbx, %rax\t# SUB"<<endl;	// substract both operands
				break;
			default:
				Error("opérateur additif inconnu");
		}
		cout << "\tpush %rax"<<endl;			// store result
	}
	return type1;

}

// DeclarationPart := "[" Ident {"," Ident} "]"
void DeclarationPart(void){
	if(current!=RBRACKET)
		Error("caractère '[' attendu");
	cout << "\t.data"<<endl;
	cout << "\t.align 8"<<endl;
	
	current=(TOKEN) lexer->yylex();
	if(current!=ID)
		Error("Un identificater était attendu");
	cout << lexer->YYText() << ":\t.quad 0"<<endl;
	DeclaredVariables.insert(lexer->YYText());
	current=(TOKEN) lexer->yylex();
	while(current==COMMA){
		current=(TOKEN) lexer->yylex();
		if(current!=ID)
			Error("Un identificateur était attendu");
		cout << lexer->YYText() << ":\t.quad 0"<<endl;
		DeclaredVariables.insert(lexer->YYText());
		current=(TOKEN) lexer->yylex();
	}
	if(current!=LBRACKET)
		Error("caractère ']' attendu");
	current=(TOKEN) lexer->yylex();
}

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
OPREL RelationalOperator(void){
	OPREL oprel;
	if(strcmp(lexer->YYText(),"==")==0)
		oprel=EQU;
	else if(strcmp(lexer->YYText(),"!=")==0)
		oprel=DIFF;
	else if(strcmp(lexer->YYText(),"<")==0)
		oprel=INF;
	else if(strcmp(lexer->YYText(),">")==0)
		oprel=SUP;
	else if(strcmp(lexer->YYText(),"<=")==0)
		oprel=INFE;
	else if(strcmp(lexer->YYText(),">=")==0)
		oprel=SUPE;
	else 
		oprel=WTFR;
	current=(TOKEN) lexer->yylex();
	return oprel;
}

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
//  récupère le type de SimpleExpression, vérifie que tous les appels suivants à 
//  SimpleExpression renvoient le même type (ou un type compatible dans le futur). 
//  Sinon, Expression doit générer une erreur. Si aucun opérateur relationnel 
//  n'a été lu, Expression renvoie le type retourné par SimpleExpression, sinon, 
//  elle renvoie le type BOOLEAN.
enum TYPES Expression(void){
	enum TYPES type1, type2;
	OPREL oprel;
	type1 = SimpleExpression();
	if(current==RELOP){
		oprel = RelationalOperator();
		type2 = SimpleExpression();
		if( type2 != type1 ){
			Error("types incompatibles pour la comparaison");
		}
		cout << "\tpop %rax"<<endl;
		cout << "\tpop %rbx"<<endl;
		cout << "\tcmpq %rax, %rbx"<<endl;
		switch(oprel){
			case EQU:
				cout << "\tje Vrai"<<++TagNumber<<"\t# If equal"<<endl;
				break;
			case DIFF:
				cout << "\tjne Vrai"<<++TagNumber<<"\t# If different"<<endl;
				break;
			case SUPE:
				cout << "\tjae Vrai"<<++TagNumber<<"\t# If above or equal"<<endl;
				break;
			case INFE:
				cout << "\tjbe Vrai"<<++TagNumber<<"\t# If below or equal"<<endl;
				break;
			case INF:
				cout << "\tjb Vrai"<<++TagNumber<<"\t# If below"<<endl;
				break;
			case SUP:
				cout << "\tja Vrai"<<++TagNumber<<"\t# If above"<<endl;
				break;
			default:
				Error("Opérateur de comparaison inconnu");
		}
		cout << "\tpush $0\t\t# False"<<endl;
		cout << "\tjmp Suite"<<TagNumber<<endl;
		cout << "Vrai"<<TagNumber<<":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True"<<endl;	
		cout << "Suite"<<TagNumber<<":"<<endl;
		return BOOLEAN;
	}
	return type1;
}

// AssignementStatement := Identifier ":=" Expression
// il faut vérifier que la variable et l'expression sont du même type et si ce n'est 
// pas le cas, générer une erreur. Dans les instructions IF et WHILE, il faut vérifier 
// que l'expression est bien de type BOOLEAN et sinon, générer une erreur.
void AssignementStatement(void){
	enum TYPES type1, type2;
	string variable;
	if(current!=ID)
		Error("Identificateur attendu");
	if(!IsDeclared(lexer->YYText())){
		cerr << "Erreur : Variable '"<<lexer->YYText()<<"' non déclarée"<<endl;
		exit(-1);
	}
	variable=lexer->YYText();
	type1 = UNSIGNED_INT;
	current=(TOKEN) lexer->yylex();
	if(current!=ASSIGN)
		Error("caractères ':=' attendus");
	current=(TOKEN) lexer->yylex();
	type2 = Expression();
	if( type2 != type1 ){
		cerr << "Type variable " << type1 <<endl;
		cerr << "Type Expression " << type2 <<endl;
		Error("types incompatibles dans l'affectation");
	}
	cout << "\tpop "<<variable<<endl;
}

// (ajout) IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
void IfStatement(void){
	current = (TOKEN)lexer->yylex();
	Expression();
	if( current!=KEYWORD || strcmp(lexer->YYText(), "THEN")!=0 ){
		Error("mot cle THEN attendu");
	}
	current = (TOKEN)lexer->yylex();
	Statement();
	if( current==KEYWORD || strcmp(lexer->YYText(), "ELSE")==0 ){
		current = (TOKEN)lexer->yylex();
		Statement();
	}
}

// (ajout) WhileStatement := "WHILE" Expression DO Statement
void WhileStatement(void){
	current = (TOKEN)lexer->yylex();
	Expression();
	if( current!=KEYWORD || strcmp(lexer->YYText(), "DO")!=0 ){
		Error("mot cle DO attendu");
	}
	current = (TOKEN)lexer->yylex();
	Statement();
}

// (ajout) ForStatement := "FOR" AssignementStatement "To" Expression "DO" Statement
void ForStatement(void){
	current = (TOKEN)lexer->yylex();
	AssignementStatement();
	if( current!=KEYWORD || strcmp(lexer->YYText(), "TO")!=0 ){
		Error("mot cle TO attendu");
	}
	current = (TOKEN)lexer->yylex();
	Expression();
	if( current!=KEYWORD || strcmp(lexer->YYText(), "DO")!=0 ){
		Error("mot cle DO attendu");
	}
	current = (TOKEN)lexer->yylex();
	Statement();
}

// (ajout) BlockStatement := "BEGIN" Statement { ";" Statement } "END"
void BlockStatement(void){
	current = (TOKEN)lexer->yylex();
	Statement();
	while( current == SEMICOLON ){
		current = (TOKEN)lexer->yylex();
		Statement();
	}
	if( current!=KEYWORD || strcmp(lexer->YYText(), "END")!=0 ){
		Error("mot cle END attendu");
	}
	current = (TOKEN)lexer->yylex();
}

// Statement := AssignementStatement
//(modifed) Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement
void Statement(void){
	// AssignementStatement();
	if( current == KEYWORD ){
		if( strcmp(lexer->YYText(), "IF") == 0 ){
			IfStatement();
		}
		else if( strcmp(lexer->YYText(), "WHILE") == 0 ){
			WhileStatement();
		}
		else if( strcmp(lexer->YYText(), "FOR") == 0 ){
			ForStatement();
		}
		else if( strcmp(lexer->YYText(), "BEGIN") == 0 ){
			BlockStatement();
		}
		else{
			Error("mot cle inconnu");
		}
	}
	else{
		AssignementStatement();
	}
}

// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
	Statement();
	while(current==SEMICOLON){
		current=(TOKEN) lexer->yylex();
		Statement();
	}
	if(current!=DOT)
		Error("caractère '.' attendu");
	current=(TOKEN) lexer->yylex();
}

// Program := [DeclarationPart] StatementPart
void Program(void){
	if(current==RBRACKET)
		DeclarationPart();
	StatementPart();	
}

int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	// Let's proceed to the analysis and code production
	current=(TOKEN) lexer->yylex();
	Program();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(current!=FEOF){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error("."); // unexpected characters at the end of program
	}

}
		
			





