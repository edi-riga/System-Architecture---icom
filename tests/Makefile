CC:=gcc
CXX:=g++
CROSS_COMPILE?=

CFLAGS?=
LFLAGS?=

OUT:=out/main.elf
INC:=-Iinc
INC+=-I../inc
LIB:=-lgtest
LIB+=-L../out -l:libicom.a

SRC:=$(shell find src -name "*.c" -o -name "*.cpp")
OBJ:=$(SRC:src/%=obj/%.o)
DEP:=$(OBJ:obj/%.o=dep/%.d)

DIR:=$(sort $(dir $(OBJ) $(DEP) $(OUT)))

all: $(DIR) $(OUT) libs

$(DIR):
	mkdir -p $@

$(OUT): $(OBJ) ../out/libicom.a
	$(CROSS_COMPILE)$(CXX) $(LFLAGS) $(OBJ) -o $(OUT) $(LIB)

obj/%.c.o: src/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(INC) -MMD -MP -MF $(patsubst obj/%.o,dep/%.d,$@) -c -o $@ $<

obj/%.cpp.o: src/%.cpp
	$(CROSS_COMPILE)$(CXX) $(CFLAGS) $(INC) -MMD -MP -MF $(patsubst obj/%.o,dep/%.d,$@) -c -o $@ $<

libs:
	make -C ../

clean:
	rm -fr $(DIR) tags

ctags:
	ctags --language-force=c -R -o tags .

-include $(DEP)
