########## DEFINITIONS #################
exten = $(addsuffix $(1),$(basename $(2)))
object = $(addprefix $(BUILD)/,$(call exten,.o,$(subst /,-,$(subst ./,,$(subst ../,,$(1))))))

EXTENS			:= .c .C .cc .c++ .cpp .cxx
BUILD			?= tmp
SOURCES			+= $(foreach d,$(SUBDIRS),$(foreach e,$(EXTENS),$(wildcard $(d)/*$(e))))
SOURCES			:= $(filter-out $(EXCLUDES),$(SOURCES))
OBJECTS			+= $(call object,$(SOURCES))
DEPENDENCIES	:= $(call exten,.d,$(OBJECTS))

CXX				?= g++
CXXFLAGS		?= -std=c++14 -O0 -g -Wall
LDFLAGS			?= -Wl,-O1

########## FUNCTIONS ###################
remove = $(filter-out $(addprefix %,$(call exten,.o,$(filter-out $(1),$(TARGETS)))),$(OBJECTS))

define target =
ifeq (.a,$(suffix $(1)))

$(1): $(EXTERN) $(OBJECTS)
	@echo "Archiving $(1)"
	ar rcs $(1) $(OBJECTS)
	@echo

else ifeq (.so,$(suffix $(TARGET)))

CXXFLAGS += -fpic

$(1): $(EXTERN) $(OBJECTS)
	@echo "Creating $(1)"
	$(CXX) -shared -o $(1) $(OBJECTS) $(LDFLAGS) $(LIBRARIES)
	@echo

else

$(1): $(EXTERN) $(call remove,$(1))
	@echo "Linking $(1)"
	$(CXX) -o $(1) $(call remove,$(1)) $(LDFLAGS) $(LIBRARIES)
	@echo

endif
endef

########## TARGET RULES ################
ifeq (,$(TARGETS))

all: $(EXTERN) $(OBJECTS)
	@echo "Created $(OBJECTS)"
	@echo

else

all: $(TARGETS)
	@echo "Created $(TARGETS)"
	@echo

$(foreach t,$(TARGETS),$(eval $(call target,$(t))))

endif

########## COMPILE RULES ###############
define compile =
$(1): $(2)
	@echo "Compiling $(1)"
	@mkdir -p $(dir $(1))
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -MMD -o $(1) -c $(2)
endef

$(foreach s,$(SOURCES),$(eval $(call compile,$(call object,$(s)),$(s))))

########## DEPENDENCIES ################
ifneq (clean,$(MAKECMDGOALS))
ifneq (distclean,$(MAKECMDGOALS))
-include $(DEPENDENCIES)
endif
endif

########## CLEAN RULES #################
clean:
	@echo "Cleaning files"
	rm -f $(OBJECTS) $(DEPENDENCIES)
ifneq (,$(wildcard $(BUILD)))
	@echo
	@echo "Removing empty dirs"
	find $(BUILD) -type d -empty -delete
endif
	@echo

distclean: clean
	@echo "Removing $(TARGETS)"
	rm -f $(TARGETS)
	@echo

.PHONY: all clean distclean
