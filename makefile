# define some variables

TARGET = trecubing # name of executable
TARGETOMP := $(addsuffix omp, $(TARGET))

# folders 
BUILDDIR = build
SRCDIR = src


# initialise some vairables for implicit C compilations
CC = gcc-13
CFLAGS = -Ofast -march=native -I/usr/local/include -pedantic
LIBRARIES = -L/usr/local/lib -lgmp -largp -lcrypto
LIBRARIESOMP = -L/usr/local/lib -largp -lcrypto -lgmpomp


# here we would put extra dependencies if needed.
# example main.o : main.c testTimes.o --> meaning that we need to rebuild main.o every ttime main.c or testTimes.o changes
all: $(TARGET) $(TARGETOMP)

testTimes.o : $(apprefix $(SRCDIR)/, fpe.h delay.h enc.h rand.h)

fpe.o : $(SRCDIR)/rand.h

delay.o : $(SRCDIR)/fpe.h

main.o : $(addprefix $(SRCDIR)/, testTimes.h constructPrimes.h)



#########################################################################################################################
# DEFAULT RULES
#  - build target by linking every object
#  - build object files in BUILDDIR

SOURCES := $(shell find $(SRCDIR) -name '*.c')

# /real/ list of objects file after prepending the build directory
OBJECTS := $(subst $(SRCDIR),$(BUILDDIR),$(SOURCES:%.c=%.o))


# build objects in the BUILDIR using the source files in SRCDIR
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR):
	mkdir -p $@

# to create the test program we need all objects and we simply link them
$(TARGET) : $(OBJECTS)
	$(CC) $(CLFAGS)  -o $(TARGET) $(OBJECTS) $(LIBRARIES)

$(TARGETOMP) : $(OBJECTS)
	$(CC) $(CFLAGS) -fopenmp -o $(TARGETOMP) $(OBJECTS) $(LIBRARIESOMP)



# we have a "phony" target clean (menaing that clean is not a file to be created
# clean will simply remove test and all object files
.PHONY: clean all
clean :
	rm  $(BUILDDIR)/*.o $(TARGET)

