# ------------------------------------------------------------------------
# Globals
# ------------------------------------------------------------------------

NAME = lswifi
VERSION = 0.8.5

SRC = main.c
OBJ = $(SRC:.c=.o)
BIN = $(NAME)

OPTGEN=tools/options.sh

MANPAGE=$(NAME).1
README=Readme.md

RELEASE=$(NAME)-$(VERSION).tar.gz

RELEASE_FILES=COPYING Makefile Readme.md config.mk lswifi.1 main.c

.include <config.mk>

.SUFFIXES: .c .o

# ------------------------------------------------------------------------
# Entry Point
# ------------------------------------------------------------------------

all: $(BIN)

# ------------------------------------------------------------------------
# Doc
# ------------------------------------------------------------------------

$(README): $(MANPAGE)
	mandoc -T markdown $(MANPAGE) > $(README)
	
# ------------------------------------------------------------------------
# Compile
# ------------------------------------------------------------------------

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	rm -f $(BIN)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

# ------------------------------------------------------------------------
# Management
# ------------------------------------------------------------------------

install: $(BIN) $(MANPAGE)
	$(INSTALLSCRIPT) $(BIN) $(PREFIX)/bin/$(BIN)
	$(INSTALLDATA)   $(MANPAGE) $(PREFIX)/man/man1/$(MANPAGE)
	
clean:
	rm -f $(OBJ)
	rm -f $(BIN)
	rm -f $(RELEASE)
	rm -f Makefile.bak
	rm -f Readme.html # not Readme.md !
	
release: $(README)
	tar czf $(RELEASE) $(RELEASE_FILES)

.PHONY:: install all clean

# ------------------------------------------------------------------------
# Development
# ------------------------------------------------------------------------
	
# run a very simple example
run: $(BIN)
	./$(BIN)
	
deps: depend
depend: options.h options.c
	makedepend -- $(CFLAGS) -- $(SRC)
	
man: $(MANPAGE)
	mandoc -a $(MANPAGE)
	
.PHONY:: man run depend doc deps

# ------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------
#
# These are built by calling 'make depend'.
# Do not modify below.
#

# DO NOT DELETE
