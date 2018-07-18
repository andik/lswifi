# Where is your X11 installed?
X11DIR=/usr/X11R6

# make install target
PREFIX=/usr/local

# will be called upon 'make install' 	like $(INSTALL) src dest 
INSTALLSCRIPT=install -s -o root -g bin -m 755
INSTALLDATA=install -o root -g bin -m 644

# Compiler and Linker Flags
CINCL   = 
CFLAGS  = -pedantic -O2 -g $(CINCL)
LDFLAGS = -lm

