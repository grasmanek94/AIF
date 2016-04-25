# This file demonstrates how to compile the AIF project on Linux.
#
# To compile AIF do:
#
# make AIF 
#

GPP = g++
GCC = gcc
AIF_OUTFILE = "./AIF.so"

COMPILE_FLAGS = -DZIP_STD -std=c++0x -fPIC -c -w -D LINUX -D PROJECT_NAME=\"AIF\"

AIF = -D AIF $(COMPILE_FLAGS)

all: AIF

clean:
	-rm -f *~ *.o *.so

AIF: clean
	$(GPP) $(AIF) main.cpp
	$(GPP) $(AIF) ZIP/unix/unzip.cpp
	$(GPP) -m32 -fshort-wchar -shared -o $(AIF_OUTFILE) *.o

