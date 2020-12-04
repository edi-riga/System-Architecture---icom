DIRS:=$(shell ls -d */)
DIRS:=$(subst testUtils/,,$(DIRS))
TESTS:=$(subst /,/out/main.elf,$(DIRS))
DIRS_CLEAN:=$(subst /,,$(DIRS))
DIRS_CLEAN:=$(addsuffix clean,$(DIRS_CLEAN))

.PHONY: $(DIRS)


all: $(TESTS)

compile: $(DIRS)

# run tests
$(TESTS): $(DIRS)
	eval "./$@";

# compile tests
$(DIRS):
	+$(MAKE) -C $@

# clean tests
clean: $(DIRS_CLEAN)

%clean: % 
	+$(MAKE) -C $< clean
