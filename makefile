TARGET = trecubing # name of executable

# folders
BUILDDIR = build
SRCDIR = src


# initialise some vairables for implicit C compilations
CC = gcc-14
CFLAGS = -Ofast -march=native -I/usr/local/include -pedantic
LIBRARIES = -L/usr/local/lib -lgmp -largp -lcrypto


all: $(TARGET)

testTimes.o : $(apprefix $(SRCDIR)/, enc.h rand.h constructPrimes.h hash.h)

main.o : $(addprefix $(SRCDIR)/, testTimes.h constructPrimes.h)

#########################################################################################################################
# DEFAULT RULES
SOURCES := $(shell find $(SRCDIR) -name '*.c')

OBJECTS := $(subst $(SRCDIR),$(BUILDDIR),$(SOURCES:%.c=%.o))


$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR):
	mkdir -p $@


$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBRARIES)


# clean will simply remove test and all object files
.PHONY: clean all
clean :
	rm -f $(BUILDDIR)/*.o $(TARGET) $(TARGETOMP)
