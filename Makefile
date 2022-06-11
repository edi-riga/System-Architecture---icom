# by default we should rely on gcc
ifneq (,$(filter $(CC), c99 cc))
CC:=gcc
else
CC?=gcc
endif

CXX?=g++
CROSS_COMPILE?=

CFLAGS:=-Wall -fPIC -g -O2
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
LIB:=
LIB_:=$(LIB) -lzmq

.PHONY: tests

DEFINES?=

all:$(DIR) $(OUT) done

done:
	@echo
	@echo "Librares: $(OUT)"
	@echo "Tests:    tests"

tests:
	@make -C tests

$(DIR):
	mkdir -p $@

out/libicom.a:$(OBJ)
	$(CROSS_COMPILE)$(AR) $(AFLAGS) $@ $(OBJ)

out/libicom.so:$(OBJ)
	$(CROSS_COMPILE)$(CC) -o $@ $(OBJ) -shared $(LIB_)

obj/%.o: src/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $<

obj/%.o: src/%.cpp
	$(CROSS_COMPILE)$(CXX) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $<

obj/ini/%.o: lib/ini/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $<

clean:
	rm -fr $(DIR)
	@make clean -C tests
