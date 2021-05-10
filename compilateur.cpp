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





// <1>
// ArithmeticExpression := Term {AdditiveOperator Term}
// Expression := <ArithmeticExpression> | <ArithmeticExpression> <RelationalOperator> <ArithmeticExpression>
// Term := Digit | "(" ArithmeticExpression ")"
// AdditiveOperator := "+" | "-"
// RelationalOperator := '=' | '=' | '!' | '<' | '>' |'<' | '>' 
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"


// <2>
// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Letter {"," Letter} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter "=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor
// Number := Digit{Digit}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"|"A"|...|"Z"




#include <string>
#include <iostream>
#include <cstdlib>
#include <set>

using namespace std;

char current, lookedAhead;				// Current char	
int NLookedAhead=0, Nchar=0, nbArith=0;
set<string> DeclaredVariables;


bool IsDeclared(char c){
	return DeclaredVariables.find(string(1,c))!=DeclaredVariables.end();
	//.find()返回查找元素的位置, .end()返回最后一个元素的下一个位置
}

void ReadChar(void){
    if(NLookedAhead>0){
        current=lookedAhead;    // Char has already been read
        NLookedAhead--;
    }
    else{

        // Read character and skip spaces until 
        // non space character is read
        while(cin.get(current) && (current==' '||current=='\t'||current=='\n'));
		Nchar++;
	

	}
	cerr<<current;
}

void LookAhead(void){
    while(cin.get(lookedAhead) && (lookedAhead==' '||lookedAhead=='\t'||lookedAhead=='\n'));
    NLookedAhead++;
	Nchar++;
}



void Error(string s){
	cerr<< s << endl;
	exit(-1);
}


// <1>
//<1>  AdditiveOperator := "+" | "-"
//<2>  AdditiveOperator := "+" | "-" | "||"	
void AdditiveOperator(void){
	if(current=='+'||current=='-')
		ReadChar();
	else{
		if( current=='|' ){
			ReadChar();
			if( current!='|' )
				Error("l'operateur de comparaison s'ecrit '||' ");
			else
				ReadChar();
		}
		else{
			Error("Opérateur additif attendu"+to_string(Nchar)+current);	   // Additive operator expected
		}
	}
}

//<1>  RelationalOperator := '=' | '=' | '!' | '<' | '>' |'<' | '>' 
//<2>  RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
void RelationalOperator(void){
	if(current=='>' || current=='=' || current=='!'){
		ReadChar();
		if( current=='='){
			ReadChar();
		}
	}
	else
		if( current=='<' ){
			ReadChar();
			if( current=='=' || current=='>' ){
				ReadChar();
			}
			// else{
			// 	Error("Opérateur relation attendu"+to_string(Nchar)+current);
			// }
		}
		else{
			Error("Opérateur relation attendu"+to_string(Nchar)+current);	 
		}
}

// Letter := "a"|...|"z"|"A"|...|"Z"
void Letter(void){
	if( current<'a' || current>'z' ){
		Error("lettre attendue");
	}
	else{
		cout << "\tpush" << current << endl;
		ReadChar();
	}
}

//<1>  Digit := "0|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"	
//<2>  Digit := 0|1|2|3|4|5|6|7|8|9	
int Digit(void){
	return current-'0';
}

// Number := Digit{Digit}
void Number(void){
	unsigned long long number;	//与long long整型相关的一共有3个：LLONG_MIN、LLONG_MAX 和 ULLONG_MIN，它们分别代表了平台上最小的long long值、最大的long long值，以及最大的unsigned long long值。	
	if( current<'0' || current>'9' )
		Error("chiffre attendu");
	else{
		number=Digit();		// ASCII 码
		ReadChar();
	}
	while( current>='0' && current<='9' ){
		number*=10;
		number+=Digit();
		ReadChar();
	}
	cout << "\tpush $" << number << endl;
}

void Expression(void);

// Factor := Number | Letter | "(" Expression ")"| "!" Factor
void Factor(void){
	if(current=='('){
		ReadChar();
		Expression();
	}
	else{
		if(current>='0' && current<='9'){
			Number();
		}
		else{

		}
	}
}

//MultiplicativeOperator := "*" | "/" | "%" | "&&"
void MultiplicativeOperator(void){
	if( current=='*' || current=='/' || current=='%' ){
		ReadChar();
	}
	else{
		if( current=='&' ){
			ReadChar();
			if( current!='&' ){
				Error("l'operateur ET s'ecrit '&&' ");
			}
			else{
				ReadChar();
			}
		}
		else{
			Error("Operateur multiplication attendu");
		}
	}
}

void ArithmeticExpression(void);			// Called by Term() and calls Term()

//<1>  Term := Digit | "(" ArithmeticExpression ")"
//<2>  Term := Factor {MultiplicativeOperator Factor}
void Term(void){
	if(current=='('){
		ReadChar();
		ArithmeticExpression();
		if(current!=')')
			Error("')' était attendu"+to_string(Nchar)+current);		// ")" expected
		else
			ReadChar();
	}
	else 
		if (current>='0' && current <='9')
			Digit();
		//else if()	//添加赋值函数变量
	    else	
			Error("'(' ou chiffre attendu "+to_string(Nchar)+current+to_string(NLookedAhead));
}

// ArithmeticExpression := Term {AdditiveOperator Term}
void ArithmeticExpression(void){
	char adop;
	Term();
	while(current=='+'||current=='-'){
		adop=current;		// Save operator in local variable
		AdditiveOperator();
		Term();
		cout << "Arith"<<nbArith<<":"<<endl;
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		if(adop=='+')
			cout << "\taddq	%rbx, %rax"<<endl;	// add both operands
		else
			cout << "\tsubq	%rbx, %rax"<<endl;	// substract both operands
		cout << "\tpush %rax"<<endl;			// store result
		cout << "FinArith"<<nbArith<<": "<<endl;
		nbArith+=1;
	}

}

// SimpleExpression := Term {AdditiveOperator Term}
void SimpleExpression(void){

}

//<1>  Expression := <ArithmeticExpression> | <ArithmeticExpression> <RelationalOperator> <ArithmeticExpression>
//<2>  Expression := SimpleExpression [RelationalOperator SimpleExpression]
void Expression(void){
	char relop;
	char relop2;
	ArithmeticExpression();
	if( current=='<' || current=='>' || current=='=' || current=='!'){
		relop=current;	// Save operator in local variable
		RelationalOperator();
		LookAhead();
		relop2=current;
		ReadChar();
		if(relop2=='='||relop2=='>'){
			ArithmeticExpression();
			cout << "Exp :"<<endl;
			// cout<<"\tpop %rax"<<endl;
			cout << "\tpop %rbx"<<endl;	// get first operand
			cout << "\tpop %rax"<<endl;	// get second operand
			cout << "\tcmpq %rbx, %rax"<<endl;
			string s= "";
			s+=relop;
			s+=relop2;
			if(s == "=="){
				cout << "\tje True"<<endl;
			}else if(s == "<="){
				cout << "\tjbe True"<<endl;
			}else if(s == ">="){
				cout << "\tjae True"<<endl;
			}else if(s == "<>" || s == "!="){
				cout << "\tjne True"<<endl;
			}else{
				Error("Opérateur relational inconnu"+to_string(Nchar)+current);
			}
			
		}
		else{
			ArithmeticExpression();
			cout << "Exp :"<<endl;
			// cout<<"\tpop %rax"<<endl;
			cout << "\tpop %rbx"<<endl;	// get first operand
			cout << "\tpop %rax"<<endl;	// get second operand
			cout << "\tcmpq %rbx, %rax"<<endl;
			if(relop == '<'){
				cout << "\tjb True"<<endl;
			}else if(relop == '>'){
				cout << "\tja True"<<endl;
			}else{
				Error("Opérateur relational inconnu"+to_string(Nchar)+current);
			}
		}
		cout << "False:"<<endl;
		cout << "\tpush $0"<<endl;
		cout << "\tjmp FinExp"<<endl;
		cout << "True:"<<endl;	//
		cout << "\tpush $1"<<endl;
		cout << "FinExp:"<<endl;
		
		
	}
}


// <2>


// DeclarationPart := "[" Letter {"," Letter} "]"
void DeclarationPart(void){
	if(current!='[')
		Error("caractere '[' attendu");
	cout << "\t.data" << endl;
	cout << "\t.align 8" <<endl;

	ReadChar();
	if(current<'a' || current>'z')
		Error("Une lettre etait attendue");
	cout << current << ":\t.quad 0" << endl;
	DeclaredVariables.insert(string(1,current));
	ReadChar();
	while(current==','){
		ReadChar();
		if(current<'a' || current>'z')
			Error("2Une lettre etait attendue");
		cout << current << ":\t.quad 0" << endl;
		DeclaredVariables.insert(string(1,current));
		ReadChar();
	}
	if(current!=']')
		Error("Caractere ']' attendu");
	ReadChar();
}

// AssignementStatement := Letter "=" Expression
void AssignementStatement(void){
	char letter;
	if(current<'a' || current>'z')
		Error("lettre attendue");
	letter=current;
	if(!IsDeclared(letter)){
		cerr << "Erreur : Variable '"<<letter<<"' non declaree"<<endl;
		exit(-1);
	}
	ReadChar();
	if(current!='=')
		Error("caractere '=' attendu");
	ReadChar();
	Expression();
	cout << "\tpop "<<letter<<endl;
}

// Statement := AssignementStatement
void Statement(void){
	AssignementStatement();
}

// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
	Statement();
	while(current==';'){
		ReadChar();
		Statement();
	}
	if(current!='.')
		Error("caractere '.' attendu");
	ReadChar();
}

// Program := [DeclarationPart] StatementPart
void Program(void){
	if(current=='['){
		DeclarationPart();
	}
	StatementPart();
}


int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	// cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	// cout << "\t.text\t\t# The following lines contain the program"<<endl;
	// cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	// cout << "main:\t\t\t# The main function body :"<<endl;
	// cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;

	// Let's proceed to the analysis and code production
	ReadChar();
	Program();
	// Expression();
	// ReadChar();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(cin.get(current)){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error("."); // unexpected characters at the end of program
	}

}
		
			





