##############################################################################################
# This Makefile was automatically generated from a Flow Studio project
# Do not edit manually
##############################################################################################


CC = ppc-morphos-gcc-4
CFLAGS = -noixemul  -Wall -Wextra
CXX = ppc-morphos-g++-4
CPPFLAGS = -noixemul
OBJCC = ppc-morphos-clang
OBJCFLAGS = -fobjc-runtime=objfw -fconstant-string-class=OBConstantString
OBJCINCLUDES = -I/SDK/Frameworks/include
CFLAGS += -MD -MP
CPPFLAGS += -MD -MP
OBJCFLAGS += -MD -MP

ifdef GCC_FLOWCOLORS
export GCC_COLORS = $(GCC_FLOWCOLORS)
export TERM = xterm
endif

LD=ppc-morphos-gcc-4
LDFLAGS = -noixemul

AR=ppc-morphos-ar
RANLIB=ppc-morphos-ranlib

CLIB_DEBUG = -noixemul

SOURCES = main.c

C_SOURCES := $(filter %.c,$(SOURCES))
CXX_SOURCES := $(filter %.cpp %.cxx %.cc,$(SOURCES))
OBJC_SOURCES := $(filter %.m,$(SOURCES))

.PHONY: all
all: DEBUG

OBJSDIR_DEBUG = 
EXECDIR_DEBUG = 
C_OBJS_DEBUG=$(C_SOURCES:%.c=$(OBJSDIR_DEBUG)%_DEBUG.o)
CXX_OBJS_DEBUG=$(patsubst %.cxx,$(OBJSDIR_DEBUG)%_DEBUG.o,$(patsubst %.cc,$(OBJSDIR_DEBUG)%_DEBUG.o,$(patsubst %.cpp,$(OBJSDIR_DEBUG)%_DEBUG.o,$(CXX_SOURCES))))
OBJC_OBJS_DEBUG=$(OBJC_SOURCES:%.m=$(OBJSDIR_DEBUG)%_DEBUG.mo)
ALL_OBJS_DEBUG = $(C_OBJS_DEBUG) $(CXX_OBJS_DEBUG) $(OBJC_OBJS_DEBUG)
ALL_OBJDIRS_DEBUG = $(dir $(ALL_OBJS_DEBUG))

.PHONY: MKDIR_DEBUG
MKDIR_DEBUG:
	 @mkdir -p $(ALL_OBJDIRS_DEBUG)

$(OBJSDIR_DEBUG)%_DEBUG.o : %.c ; @echo compiling $@ ; $(CC) $(CLIB_DEBUG) -c $(CFLAGS) $(CINCLUDES) $(CDEFINES) $< -o $@
$(OBJSDIR_DEBUG)%_DEBUG.o : %.cxx ; @echo compiling $@ ; $(CXX) $(CLIB_DEBUG) -c $(CFLAGS) $(CPPINCLUDES) $(CPPDEFINES) $< -o $@
$(OBJSDIR_DEBUG)%_DEBUG.o : %.cc ; @echo compiling $@ ; $(CXX) $(CLIB_DEBUG) -c $(CPPFLAGS) $(CPPINCLUDES) $(CPPDEFINES) $< -o $@
$(OBJSDIR_DEBUG)%_DEBUG.o : %.cpp ; @echo compiling $@ ; $(CXX) $(CLIB_DEBUG) -c $(CPPFLAGS) $(CPPINCLUDES) $(CPPDEFINES) $< -o $@
$(OBJSDIR_DEBUG)%_DEBUG.mo : %.m ; @echo compiling $@ ; $(OBJCC) $(CLIB_DEBUG) -c $(OBJCFLAGS) $(OBJCINCLUDES) $(OBJCDEFINES) $< -o $@

EXECNAME_DEBUG = TestAmbient_DEBUG
.PHONY: DEBUG
DEBUG: MKDIR_DEBUG  $(EXECNAME_DEBUG) 
	
$(EXECNAME_DEBUG): $(ALL_OBJS_DEBUG) 
	@echo linking $@ ;$(LD) $(CLIB_DEBUG) $(LDFLAGS) $(ALL_OBJS_DEBUG) $(LINKLIBS) -o $@

-include $(patsubst %.c,$(OBJSDIR_DEBUG)%_DEBUG.d,$(C_SOURCES))
-include $(patsubst %.cxx,$(OBJSDIR_DEBUG)%_DEBUG.d,$(patsubst %.cc,$(OBJSDIR_DEBUG)%_DEBUG.d,$(patsubst %.cpp,$(OBJSDIR_DEBUG)%_DEBUG.d,$(CXX_SOURCES))))
-include $(patsubst %.m,$(OBJSDIR_DEBUG)%_DEBUG.d,$(OBJC_SOURCES))



.PHONY: clean
clean:
	rm -f $(EXECDIR_DEBUG)$(EXECNAME_DEBUG) $(OBJSDIR_DEBUG)*.o $(OBJSDIR_DEBUG)*.mo $(OBJSDIR_DEBUG)*.d

