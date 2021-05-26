VAR     a,i:INTEGER;
        b,s:DOUBLE;
        c:BOOLEAN;
        o:CHAR.
a:=0;
b:=1.2;
c:=3>4;
b:=b*1.1;
s:=2.5;
o:='c';
WHILE a<3 DO
BEGIN
        IF a==5 THEN
                a:=a+2
        ELSE
                a:=a+1;
        a:=a+1
END;
IF a==10 THEN
        a:=a*2
ELSE
        a:=a+1;
FOR i:=1 TO 2 DO
BEGIN
        s:=s*0.99
END;
DISPLAY s;
DISPLAY c.