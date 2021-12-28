CC:=gcc
CXX:=g++
CROSS_COMPILE?=
DEFINES?=

CFLAGS?=-g -std=gnu11
LFLAGS?=

OUT:=out/main.elf
INC:=-Iinc
INC+=-I../inc
LIB:=-lpthread
LIB+=-L../out -l:libicom.a

SRC:=$(shell find src -name "*.c" -o -name "*.cpp")
OBJ:=$(SRC:src/%=obj/%.o)
DEP:=$(OBJ:obj/%.o=dep/%.d)

DIR:=$(sort $(dir $(OBJ) $(DEP) $(OUT)))

all: libs $(DIR) $(OUT)

$(DIR):
	mkdir -p $@

$(OUT): ../out/libicom.a $(OBJ)
	$(CROSS_COMPILE)$(CC) $(LFLAGS) $(OBJ) -o $(OUT) $(LIB)

obj/%.c.o: src/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(INC) -MMD -MP -MF $(patsubst obj/%.o,dep/%.d,$@) -c -o $@ $<

obj/%.cpp.o: src/%.cpp
	$(CROSS_COMPILE)$(CXX) $(CFLAGS) $(INC) -MMD -MP -MF $(patsubst obj/%.o,dep/%.d,$@) -c -o $@ $<

libs:
	make DEFINES=$(DEFINES) -C ../

clean:
	rm -fr $(DIR) tags

ctags:
	ctags --language-force=c -R -o tags .

-include $(DEP)
