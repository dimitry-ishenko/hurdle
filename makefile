########## DEFINITIONS #################
TARGETS		:= hurdle

SUBDIRS		:= src util
SOURCES		:=
EXCLUDES	:=
INCLUDES	:= -I. -Isrc
LIBRARIES	:= -lcurl -pthread
DEFINES		:=
OBJECTS		:=
EXTERN		:=

########## OPTIONS #####################
#CXX			:= clang++
#CXXFLAGS	:= -std=c++14 -O3

include makefile_tgt.mk
