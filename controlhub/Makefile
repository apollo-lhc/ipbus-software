###################################################################################
#
# Simple Erlang build file with testing built-in.
# Inspiration taken from an erlang-factory.com presentation by Richard Carlsson 
#
#   Robert Frazier, March 2012
#
###################################################################################


ERLC_FLAGS=

SOURCES=$(wildcard src/*.erl)
INCLUDE_DIR=include
INCLUDES=$(wildcard $(INCLUDE_DIR)/*.hrl)
OBJECTS=$(SOURCES:src/%.erl=ebin/%.beam)

all: ebin_dir $(OBJECTS) test

ebin_dir:
	test -d $(OBJDIR) || mkdir $(OBJDIR)

ebin/%.beam : src/%.erl $(INCLUDES)
	erlc $(ERLC_FLAGS) -I $(INCLUDE_DIR) -o ebin/ $<

clean: 
	rm -rf ebin

test:
	erl -noshell -pa ebin -eval 'eunit:test("ebin",[verbose])' -s init stop

release: clean
	$(MAKE) ERLC_FLAGS="$(ERLC_FLAGS) -DNOTEST"