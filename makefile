all:	test
clean:
		rm *.o *.s
		rm tokensiser.cpp
tokeniser.cpp:	tokeniser.l				#verifier s'il y a fichie tockeniser.l, s'il liee , flex++ va creer une fichier .cpp a partir du fichier tokeniser.l	s'il liee , flex++ va creer une fichier .cpp a partir du fichier tokeniser.l	
		flex++ -d -o tokeniser.cpp tokeniser.l			
tokeniser.o:	tokeniser.cpp
		g++ -c tokeniser.cpp
compilateur:	compilateur.cpp tokeniser.o			#pour creer compilateur , il nous faut donc compilateur.cpp et tockeniser.o
		g++ -ggdb -o compilateur compilateur.cpp tokeniser.o
test:		compilateur test.p
		./compilateur <test.p >test.s
		gcc -ggdb -no-pie test.s -o test


