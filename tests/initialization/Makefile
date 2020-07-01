CC:=gcc
CPP:=g++
CROSS_COMPILE:=

CFLAGS:=
LFLAGS:=

SRC:=$(shell find . -name "*.c")
SRC+=$(shell find . -name "*.cpp")
SRC+=$(shell find ../testUtils/src/ -name "*.c")
OBJ:=$(subst ../testUtils/,./,$(SRC))
OBJ:=$(subst src/,obj/,$(OBJ))
OBJ:=$(subst .cpp,.o,$(OBJ))
OBJ:=$(subst .c,.o,$(OBJ))

OUT:=out/main.elf
DIR:=$(sort $(dir $(OBJ))) $(sort $(dir $(OUT)))
INC:=-Iinc -I../../inc -I../testUtils/inc
LIB:=-pthread -L../../out -l:libicom.a -lzmq

all:$(DIR) $(OUT)

$(DIR):
	mkdir -p $@

$(OUT):$(OBJ)
	$(CROSS_COMPILE)$(CC) $(LFLAGS) $(OBJ) -o $(OUT) $(LIB)

obj/icom.o: ../../src/icom.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(INC) -c -o $@ $<

obj/%.o: src/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(INC) -c -o $@ $<

obj/%.o: src/%.cpp
	$(CROSS_COMPILE)$(CPP) $(CFLAGS) $(INC) -c -o $@ $<

obj/%.o: ../testUtils/src/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -fr $(DIR)
