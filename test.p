VAR     a,i,s:INTEGER;
        b:DOUBLE;
        c:BOOLEAN.
a:=0;
b:=1.2;
c:=3>4;
b:=b*1.1;
s:=0;
WHILE a<10 DO
BEGIN
        IF a==5 THEN
                a:=a*2
        ELSE
                a:=a+1;
        a:=a+1
END;
IF a==10 THEN
        a:=a*2
ELSE
        a:=a+1;
FOR i:=1 TO 10 DO
BEGIN
        s:=s+i
END.