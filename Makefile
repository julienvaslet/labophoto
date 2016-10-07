applicationsDirectory := apps
binariesDirectory := bin
librariesDirectory := lib
for :=

applications := $(shell cd $(applicationsDirectory) && ls -1 *.cpp | sed 's|\.cpp$$||')
options :=

ifneq ($(for),)
	options := $(options) -t $(for)
endif

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

