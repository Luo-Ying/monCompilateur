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
#include <map>
#include <FlexLexer.h>
#include "tokeniser.h"
#include <cstring>

using namespace std;

enum OPREL {EQU, DIFF, INF, SUP, INFE, SUPE, WTFR};			//enum <类型名> {<枚举常量表>};
enum OPADD {ADD, SUB, OR, WTFA};
enum OPMUL {MUL, DIV, MOD, AND ,WTFM};
enum TYPES {UNSIGNED_INT, BOOLEAN, INTEGER, DOUBLE, CHAR, STRING};

TOKEN current;				// Current token


FlexLexer* lexer = new yyFlexLexer; // This is the flex tokeniser
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

	
// set<string> DeclaredVariables;	// Store declared variables and their types
map<string, enum TYPES> DeclaredVariables;		// Store declared variables and their types
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
	if( !IsDeclared(lexer->YYText()) ){
		cerr << "Erreur : Variable " << lexer->YYText() << " non declare" <<endl;
		exit(-1);
	}
	type = DeclaredVariables[lexer->YYText()];
	cout << "\tpush "<<lexer->YYText()<<endl;
	current=(TOKEN) lexer->yylex();
	return type;
}

// -- retourner le type UNSIGNED_INT (prévoir qu'il y aura plusieurs possibilités dans le futur) --
enum TYPES Number(void){
	double d;									// 64-bit float
	unsigned int * i;							// ponier to a 32 bit unsigned int
	string number = lexer->YYText();
	if( number.find(".") != string::npos ){		// Floating point constant number
		// string::find()函数：是一个字符或字符串查找函数，该函数有唯一的返回类型，即string::size_type，即一个无符号整形类型，可能是整数也可能是长整数。
		// 如果查找成功，返回按照查找规则找到的第一个字符或者子串的位置；
		// 如果查找失败，返回string::npos,即-1（当然打印出的结果不是-1，而是一个很大的数值，那是因为它是无符号的）。
		d = atof(lexer->YYText());			// atof()会扫描参数nptr字符串，跳过前面的空格字符，直到遇上数字或正负符号才开始做转换，而再遇到非数字或字符串结束时('\0')才结束转换，并将结果返回。
		i = (unsigned int *) &d;			// i points to the const double
		// cout << "\tpush $" << *i << "\t# Conversion of " << d <<endl;
		//Is equivalent to :
		cout << "\tsubq $8,%rsp\t\t\t# allocate 8 bytes on stack's top" <<endl;		//move the stack top (decaler le pile)
		cout << "\tmovl $" << *i << ", (%rsp)\t# Conversion of " << d << " (32 bit high part)" <<endl;	
		cout << "\tmovl $" << *(i+1) << ", 4(%rsp)\t# Conversion of " << d << " (32 bit low part)" <<endl;	// unsigned int is 32 bit
		current = (TOKEN) lexer->yylex();
		return DOUBLE;
	}
	else{	// Integer Constant
		cout << "\tpush $" << atoi(lexer->YYText()) <<endl;		
		//atoi() 函数用来将字符串转换成整数(int)，其原型为： int atoi (const char * str); ... 【返回值】返回转换后的整型数；如果str 不能转换成int 或者str 为空字符串，那么将返回0。
		current = (TOKEN)lexer->yylex();
		return INTEGER;
	}
	// cout <<"\tpush $"<<atoi(lexer->YYText())<<endl;
	// current=(TOKEN) lexer->yylex();
	// return UNSIGNED_INT;
}

enum TYPES CharConst(void){
	cout << "\tmovq $0,%rax" <<endl;
	cout << "\tmovb $" << lexer->YYText() << ",%al" <<endl;
	cout << "\tpush %rax\t# push a 64-bit version of " << lexer->YYText() <<endl;
	current = (TOKEN) lexer->yylex();
	return CHAR;
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
		case CHARCONST:
			type = CharConst();
			break;
		default:
			// cout<<lexer->YYText()<<endl;
			Error("'(', ou constante ou variable attendue.");
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
		switch(mulop){
			case AND:
				if( type2!=BOOLEAN ){
					Error("type incompatible dans l'expression");
				}
				cout << "\tpop %rbx"<<endl;	// get first operand
				cout << "\tpop %rax"<<endl;	// get second operand
				cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
				cout << "\tpush %rax\t# AND"<<endl;	// store result
				break;
			case MUL:
				if( type2!=INTEGER && type2!=DOUBLE ){
					Error("type non numerique pour la multiplication");
				}
				if( type2 == INTEGER ){
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
					cout << "\tpush %rax\t# MUL"<<endl;	// store result
				}
				else{
					cout << "\tfldl 8(%rsp)\t" <<endl;
					cout << "\tfldl (%rsp)\t# first oprand -> %st(0) ; second operand -> %st(1)" <<endl;
					cout << "\tfmulp %st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null" <<endl;
					cout << "\tfstpl 8(%rsp)" <<endl;		
					cout << "\taddq $8,%rsp\t# result on stack's top" <<endl;
				}
				break;
			case DIV:
				if( type2!=INTEGER && type2!=DOUBLE ){
					Error("type non numerique pour la division");
				}
				if( type2 == INTEGER ){
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
					cout << "\tdiv %rbx"<<endl;			// quotient goes to %rax
					cout << "\tpush %rax\t# DIV"<<endl;		// store result
				}
				else{
					cout << "\tfldl 8(%rsp)\t" <<endl;
					cout << "\tfldl (%rsp)\t# first oprand -> %st(0) ; second operand -> %st(1)" <<endl;
					cout << "\tfdivp %st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null" <<endl;
					cout << "\tfstpl 8(%rsp)" <<endl;		
					cout << "\taddq $8,%rsp\t# result on stack's top" <<endl;
				}
				break;
			case MOD:
				if( type2!=INTEGER ){
					Error("type non entier pour le modulo");
				}
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
		switch(adop){
			case OR:
				if( type2 != BOOLEAN ){
					Error("operande non booleanne pour l'operateur OR");
				}
				cout << "\tpop %rbx"<<endl;	// get first operand
				cout << "\tpop %rax"<<endl;	// get second operand
				cout << "\taddq	%rbx, %rax\t# OR"<<endl;// operand1 OR operand2
				cout << "\tpush %rax"<<endl;			// store result
				break;			
			case ADD:
				if( type2!=INTEGER && type2!=DOUBLE ){
					Error("operande non numerique pour l'addition");
				}
				if( type2==INTEGER ){
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\taddq	%rbx, %rax\t# ADD"<<endl;	// add both operands
					cout << "\tpush %rax"<<endl;			// store result
				}
				else{
					cout << "\tfldl 8(%rsp)\t" <<endl;
					cout << "\tfldl (%rsp)\t# first oprand -> %st(0) ; second operand -> %st(1)" <<endl;
					cout << "\tfaddp %st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null" <<endl;
					cout << "\tfstpl 8(%rsp)" <<endl;		
					cout << "\taddq $8,%rsp\t# result on stack's top" <<endl;
				}
				break;			
			case SUB:	
				if( type2!=INTEGER && type2!=DOUBLE ){
					Error("operande non numerique pour la soustraction");
				}
				if( type2 == INTEGER ){	
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\tsubq	%rbx, %rax\t# SUB"<<endl;	// substract both operands
					cout << "\tpush %rax"<<endl;			// store result
				}
				else{
					cout << "\tfldl 8(%rsp)\t" <<endl;
					cout << "\tfldl (%rsp)\t# first oprand -> %st(0) ; second operand -> %st(1)" <<endl;
					cout << "\tfsubp %st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null" <<endl;
					cout << "\tfstpl 8(%rsp)" <<endl;		
					cout << "\taddq $8,%rsp\t# result on stack's top" <<endl;
				}
				break;
			default:
				Error("opérateur additif inconnu");
		}
	}
	return type1;

}

enum TYPES Type(void){
	if( current != TYPE ){
		Error("type attendu");
	}
	if(strcmp(lexer->YYText(), "BOOLEAN")==0){
		return BOOLEAN;
	}
	else if(strcmp(lexer->YYText(), "INTEGER")==0){
		return INTEGER;
	}
	else if(strcmp(lexer->YYText(), "DOUBLE")==0){
		return DOUBLE;
	}
	else if(strcmp(lexer->YYText(), "CHAR")==0){
		return CHAR;
	}
	else{
		Error("type inconnu");
	}
}

// VarDeclaration := Ident {"," Ident} ":" Type
void VarDeclaration(void){
	set<string> idents;
	enum TYPES type;
	if( current != ID ){
		Error("Un identificateur etait attendu");
	}
	idents.insert(lexer->YYText());
	current = (TOKEN)lexer->yylex();
	while( current == COMMA ){
		current = (TOKEN)lexer->yylex();
		if( current != ID ){
			Error("Unidentificateur etait attendu");
		}
		idents.insert(lexer->YYText());
		current = (TOKEN)lexer->yylex();
	}
	if( current != COLON ){
		Error("caractere ':' attendu");
	}
	current = (TOKEN)lexer->yylex();
	type = Type();
	for( set<string>::iterator it=idents.begin(); it!=idents.end(); ++it ){
		switch(type){
			case BOOLEAN:
			case INTEGER:
				cout << *it << ":\t.quad 0" <<endl;
				break;
			case DOUBLE:
				cout << *it << ":\t.double 0.0" <<endl;
				break;
			case CHAR:
				cout << *it << ":\t.byte 0" <<endl;
				break;
			default:
				Error("type inconnu.");
		}
		DeclaredVariables[*it] = type;
	}
	current = (TOKEN)lexer->yylex();
}

// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
void VarDeclarationPart(void){
	current = (TOKEN)lexer->yylex();
	VarDeclaration();
	while( current == SEMICOLON ){
		current = (TOKEN)lexer->yylex();
		VarDeclaration();
	}
	if( current != DOT ){
		Error(" '.' attendu");
	}
	current = (TOKEN)lexer->yylex();
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
	unsigned long long tag;
	OPREL oprel;
	type1 = SimpleExpression();
	if(current==RELOP){
		tag = ++TagNumber;
		oprel = RelationalOperator();
		type2 = SimpleExpression();
		if( type2 != type1 ){
			Error("types incompatibles pour la comparaison");
		}
		if( type1 != DOUBLE ){
			cout << "\tpop %rax"<<endl;
			cout << "\tpop %rbx"<<endl;
			cout << "\tcmpq %rax, %rbx"<<endl;
		}
		else{
			// FLDL : double
			// FLDS : float
			cout << "\tfldl	(%rsp)\t" <<endl;					
			cout << "\tfldl 8(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)" <<endl;
			cout << "\taddq $16,%rsp\t# 2x pop nothing" <<endl;
			cout << "\tfcomip %st(1)\t\t# compare op1 and op2 -> %RFLAGS and pop" <<endl;	// fcomip : double
			cout << "\tfaddp %st(1)\t# pop nothing" <<endl;
		}
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
	//*************************
	if(!IsDeclared(lexer->YYText())){
		cerr << "Erreur : Variable '"<<lexer->YYText()<<"' non déclarée"<<endl;
		exit(-1);
	}
	variable=lexer->YYText();
	type1 = DeclaredVariables[variable];
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
	if( type1 == CHAR){
		cout << "\tpop %rax" <<endl;
		cout << "\tmovb %al," << variable <<endl;
	}
	cout << "\tpop "<<variable<<endl;
}

// (ajout) IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
void IfStatement(void){
	unsigned long long tag = TagNumber++;
	current = (TOKEN)lexer->yylex();
	if( Expression() != BOOLEAN ){
		Error("le type de l'expression doit etre BOOLEAN");
	}
	cout << "\tpop %rax\t# Get the result of expression" <<endl;
	cout << "\tcmpq $0,%rax" <<endl;
	cout << "\tje Else" << tag << "\t# if FALSE ,jump to Else" << tag <<endl;
	if( current!=KEYWORD || strcmp(lexer->YYText(), "THEN")!=0 ){
		Error("mot cle THEN attendu");
	}
	current = (TOKEN)lexer->yylex();
	Statement();
	cout << "\tjmp Next" << tag << "\t#Do not execute the else statement" <<endl;
	cout << "Else" << tag << ":" <<endl;		// Might be the same effective adress than Next;
	if( current==KEYWORD || strcmp(lexer->YYText(), "ELSE")==0 ){
		current = (TOKEN)lexer->yylex();
		Statement();
	}
	cout << "Next" << tag << ":" <<endl;
}

// (ajout) WhileStatement := "WHILE" Expression DO Statement
void WhileStatement(void){
	unsigned long long tag  = TagNumber++;
	cout << "While" << tag << ":" <<endl;
	current = (TOKEN)lexer->yylex();
	if( Expression() != BOOLEAN ){
		Error("expression booleene attendu");
	}
	cout << "\tpop %rax\t# Get the result of expression" <<endl;
	cout << "\tcmpq $0,%rax" <<endl;
	cout << "\tje suite" << tag << "\t# if FALSE, jump out of the loop" << tag <<endl;
	// Expression();
	if( current!=KEYWORD || strcmp(lexer->YYText(), "DO")!=0 ){
		Error("mot cle DO attendu");
	}
	current = (TOKEN)lexer->yylex();
	Statement();
	cout << "\tjmp While" << tag <<endl;
	cout << "suite" << tag << ":" <<endl;
}

// ForStatement := "For" ID ":=" Expression ("TO"|"DOWNTO") Expression "DO" Statement
void ForStatement(void){
	unsigned long long tag = TagNumber++;
	if( strcmp(lexer->YYText(), "FOR")!=0 ){
		Error("FOR expected");
	}
	// cout << "for" << tag << ":" <<endl;
	current = (TOKEN)lexer->yylex();
	string variable = lexer->YYText();
	AssignementStatement();
	if( strcmp(lexer->YYText(), "TO") != 0 && strcmp(lexer->YYText(), "DOWNTO") != 0){
		Error("TO or DOWNTO expected");
	}
	string keyword = lexer->YYText();
	current = (TOKEN)lexer->yylex();

	cout << "For" << tag << ":" <<endl;
	if( Expression() != INTEGER ){
		Error("le type de l'expression doit etre INTEGER");
	}
	cout << "\tpop %rax" <<endl;
	cout << "\tcmpq %rax," << variable <<endl;		// regarde si i est egale a but
	cout << "Do" << tag <<endl;
	cout << "\tje SuiteFor" << tag <<endl;		// si il a deppasse , jump vers SuitFor
	if( strcmp(lexer->YYText(), "DO")!=0 ){
		Error("DO expected");
	}
	current = (TOKEN)lexer->yylex();
	Statement();
	if( keyword == "TO" ){
		cout << "\taddq $1," << variable <<endl;
		cout << "\tpush %rax" <<endl;
		cout << "\tpop " << variable <<endl;
	}
	else if( keyword == "DOWNTO" ){
		cout << "\tsubq $1," << variable <<endl;
		cout << "\tpush %rax" <<endl;
		cout << "\tpop " << variable <<endl;;
	}
	cout << "\tjmp For" << tag <<endl;
	cout << "SuiteFor" << tag << ":" <<endl;
}

void CaseStatement(void){

    unsigned long long tag=TagNumber++;
    unsigned long long tag2=0;
    current=(TOKEN) lexer->yylex();
    TYPES type1,type2;
    cout<<"Case"<<tag<<":"<<endl;
    type1 = Expression();
    if(type1!=INTGER){
        Error("le type de l'expresion du switch ne doit pas etre boolean");
    }
    cout<<"\tpop %rax\t# Get the result of expression"<<endl;
    if(current!=KEYWORD || strcmp((lexer->YYText(),"OF")){
        Error("keyword 'of' attendu");
    }
    current=(TOKEN) lexer->yylex();
    while(current!=KEYWORD!strcmp(lexer->YYText(),"END")){

        current=(TOKEN) lexer->yylex();
        type2 = NUMBER();
        if(type2!=type1){
            error("types du case différents");
        }
        cout<<"\tpop %rax\t# Get the number"<<endl;

        cout<<"\tcomp %rax,%rbx\t# Get the number"<<endl;

        cout<<"\tjne Case"<<tag<<"-"<<tag2<<":"<<endl;

        if(current!=KEYWORD||strcmp(lexer->YYText(),":")){
            Error(" ':' attendu");
        }
        Statement();

        cout<<"\tjmp CaseEnd"<<tag<<":"<<endl; 

        cout<<"Case"<<tag<<"-"<<tag2<<endl;

        tag2++;

        if(current !=SEMICOLON){
            Error(" ';' attendu ");
        }
        current=(TOKEN) lexer->yylex();
    }

    cout<<"CaseEnd"<<tag<<":"<<endl;

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

// DisplayStatement := "DISPLAY" Expression
void DisplayStatement(void){
	enum TYPES type;
	unsigned long long tag = ++TagNumber;
	current = (TOKEN)lexer->yylex();
	type = Expression();
	switch(type){
		case INTEGER:
			cout << "\tpop %rsi\t# The value to be displayed" <<endl;
			cout << "\tmovq $FormatString1,%rdi\t# \"%llu\\n\"" <<endl;
			cout << "\tmovl $0,%eax" <<endl;
			cout << "\tcall printf" <<endl;
			// cout << "\taddl $8, %esp" <<endl;
			// cout << "\tcall exit" <<endl;
			break;
		case BOOLEAN:
			cout << "\tpop %rdx\t# Zero : False, non-zero : true" <<endl;
			cout << "\tcmpq $0,%rdx" <<endl;
			cout << "\tje False" << tag <<endl;
			cout << "\tmovq $TrueString,%rdi\t# \"TRUE\\n\"" <<endl;
			cout << "\tjmp Next" << tag <<endl;
			cout << "False" << tag << ":" <<endl;
			cout << "\tmovq $FalseString,%rdi\t# \"FALSE\\n\"" <<endl;
			cout << "Next" << tag << ":" <<endl;
			cout << "\tcall puts" <<endl;
			// cout << "\taddl $8, %esp" <<endl;
			// cout << "\tcall exit" <<endl;
			break;
		case DOUBLE:
			cout << "\tmovsd (%rsp),%xmm0\t\t# &stack top -> %xmm0" <<endl;
			cout << "\tsubq $16,%rsp\t\t# allocation for 3 additional doubles" <<endl;
			cout << "\tmovsd %xmm0,8(%rsp)" <<endl;
			cout << "\tmovq $FormatString2, %rdi\t# \"%lf\\n\"" <<endl;
			cout << "\tmovq $1,%rax" <<endl;
			cout << "\tcall printf@PLT" <<endl;
			// cout << "\tcall exit" <<endl;
			cout << "nop" <<endl;
			cout << "\taddq $24,%rsp\t\t\t# pop nothing" <<endl;
			// cout << "\tcall exit" <<endl;
			break;
		case CHAR:
			cout << "\tpop %rsi\t\t\t# get character in the 8 lowest bits of %si" <<endl;
			cout << "\tmocq $FormatString3, %rdi\t# \"%c\\n\"" <<endl;
			cout << "\tmovl $0,%eax" <<endl;
			cout << "\tcall printf" <<endl;
		default:
			Error("DISPLAY ne fonctionne pas pour ce type de donne.");
	}
}

// Statement := AssignementStatement
//(modifed) Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement
void Statement(void){
	// AssignementStatement();
	if( current == KEYWORD ){
		if( strcmp(lexer->YYText(), "DISPLAY") == 0 ){
			DisplayStatement();
		}
		else if( strcmp(lexer->YYText(), "IF") == 0 ){
			IfStatement();
		}
		else if( strcmp(lexer->YYText(), "WHILE") == 0 ){
			WhileStatement();
		}
		else if( strcmp(lexer->YYText(), "FOR") == 0 ){
			ForStatement();
		}
		else if( strcmp(lexer->YYText(), "CASE") == 0 ){
			CaseStatement();
		}
		else if( strcmp(lexer->YYText(), "BEGIN") == 0 ){
			BlockStatement();
		}
		else{
			Error("mot cle inconnu");
		}
	}
	else{
		if( current == ID ){
			AssignementStatement();
		}
		else{
			Error("instruction attendu");
		}
	}
}

// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	// cout << "\t.align" <<endl;
	// cout << "\t.thumb" <<endl;
	// cout << "\t.extern printf" <<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	// cout << "\t.thumb_func" <<endl;
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
	// if(current==RBRACKiET)
	// 	DeclarationPart();
	if( current==KEYWORD && strcmp(lexer->YYText(), "VAR")==0 ){
		VarDeclarationPart();
	}
	StatementPart();	
}

int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	cout << ".data" <<endl;
	cout << "FormatString1:\t.string \"%llu\"\t# used by printf to display 64-bit unsigned integers" <<endl;
	cout << "FormatString2:\t.string \"%lf\"\t# used by printf to display 64-bit floating point numbers" <<endl;
	cout << "FormatString3:\t.string \"%c\"\t# used by printf to display a 8-bit single character" <<endl;
	cout << "TrueString:\t.string \"TRUE\"\t# used by printf to display the boolean value TRUE" <<endl;
	cout << "FalseString:\t.string \"FALSE\"\t# used by printf to display the boolean value FALSE" <<endl;
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
		
			





