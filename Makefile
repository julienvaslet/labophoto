applicationsDirectory := apps
binariesDirectory := bin
librariesDirectory := lib

#applications := $(shell ls -1 $(applicationsDirectory)/* | xargs -n1 ./compile.sh -e | sed 's|^$(PWD)/||')
applications := $(shell cd $(applicationsDirectory) && ls -1 *.cpp | sed 's|\.cpp$$||')
options :=

all: $(applications)

%.PRECIOUS: %
.PHONY:  clean cleanlib all
.SECONDEXPANSION:

%: $(applicationsDirectory)/%.cpp compile.sh
	./compile.sh $(options) $<

cleanlib:
	rm -rf $(librariesDirectory)/*
	
clean:
	find . -name '*~' | xargs rm -f
	rm -f $(binariesDirectory)/*

