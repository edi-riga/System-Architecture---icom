CC:=gcc
CPP:=g++
CROSS_COMPILE:=

CFLAGS:=-Wall -fPIC
LFLAGS:=
AFLAGS:=rcs

SRC:=$(shell find ./src -name "*.c")
SRC+=$(shell find ./src -name "*.cpp")
SRC+=$(shell find ./lib -name "*.c")
SRC+=$(shell find ./lib -name "*.cpp")
OBJ:=$(subst src/,obj/,$(SRC))
OBJ:=$(subst lib/,obj/,$(OBJ))
OBJ:=$(subst .cpp,.o,$(OBJ))
OBJ:=$(subst .c,.o,$(OBJ))

#OUT:=out/libicom.a
OUT:=out/libicom.a out/libicom.so
DIR:=$(sort $(dir $(OBJ))) $(sort $(dir $(OUT)))
INC:=-Iinc -Ilib
LIB:=-lzmq

all:$(DIR) $(OUT) compile_tests done

done:
	@echo
	@echo "Librares: $(OUT)"
	@echo "Tests:    tests"

compile_tests:
	@make -C tests

$(DIR):
	mkdir -p $@

out/libicom.a:$(OBJ)
	$(CROSS_COMPILE)$(AR) $(AFLAGS) $@ $(OBJ)

out/libicom.so:$(OBJ)
	$(CROSS_COMPILE)$(CC) -o $@ $(OBJ) -shared $(LIB)

obj/%.o: src/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(INC) -c -o $@ $<

obj/%.o: src/%.cpp
	$(CROSS_COMPILE)$(CPP) $(CFLAGS) $(INC) -c -o $@ $<

obj/ini/%.o: lib/ini/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -fr $(DIR)
	@make clean -C tests
