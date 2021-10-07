CC?=gcc
CPP?=g++
CROSS_COMPILE?=

CFLAGS?=
CFLAGS:=-Wall -fPIC $(CFLAGS)
LFLAGS?=
LFLAGS:=$(LFLAGS)
AFLAGS:=rcs
DEFINES?=

SRC:=$(shell find ./src -name "*.c")
SRC+=$(shell find ./src -name "*.cpp")
SRC+=$(shell find ./lib -name "*.c")
SRC+=$(shell find ./lib -name "*.cpp")
OBJ:=$(subst src/,obj/,$(SRC))
OBJ:=$(subst lib/,obj/,$(OBJ))
OBJ:=$(subst .cpp,.o,$(OBJ))
OBJ:=$(subst .c,.o,$(OBJ))

OUT:=out/libicom.a out/libicom.so
DIR:=$(sort $(dir $(OBJ))) $(sort $(dir $(OUT)))
INC:=-Iinc -Ilib
LIB?=
LIB:=$(LIB) -lzmq


all: $(OUT) done

done:
	@echo
	@echo "Librares: $(OUT)"
	@echo "Tests:    tests"

compile_tests:
	@make -C tests

$(DIR):
	mkdir -p $@

out/libicom.a: $(DIR) $(OBJ)
	$(CROSS_COMPILE)$(AR) $(AFLAGS) $@ $(OBJ)

out/libicom.so: $(DIR) $(OBJ)
	$(CROSS_COMPILE)$(CC) -o $@ $(OBJ) -shared $(LIB)

obj/%.o: src/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $<

obj/%.o: src/%.cpp
	$(CROSS_COMPILE)$(CPP) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $<

obj/ini/%.o: lib/ini/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $<

clean:
	rm -fr $(DIR)
	@make clean -C tests
