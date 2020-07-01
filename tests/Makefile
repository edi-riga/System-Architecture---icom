DIRS:=$(shell ls -d */)
DIRS:=$(subst testUtils/,,$(DIRS))
TESTS:=$(subst /,/out/main.elf,$(DIRS))
DIRS_CLEAN:=$(DIRS)

.PHONY: $(DIRS)


all: $(TESTS)

compile: $(DIRS)

# run tests
$(TESTS): $(DIRS)
	eval "./$@";

# compile tests
$(DIRS):
	+$(MAKE) -C $@

# TODO: clean
# clean tests
# clean:
#	+$(MAKE) -C $(DIRS) clean
