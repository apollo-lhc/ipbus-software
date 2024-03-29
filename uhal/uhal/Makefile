BUILD_HOME = $(shell pwd)/../..

include $(BUILD_HOME)/config/Makefile.macros
include $(BUILD_HOME)/uhal/config/mfCommonDefs.mk

Project = uhal
Package = uhal/uhal
PackagePath = $(CACTUS_RPM_ROOT)/${Package}
PackageName = cactuscore-uhal-uhal

PACKAGE_VER_MAJOR = 2
PACKAGE_VER_MINOR = 6
PACKAGE_VER_PATCH = 2
PACKAGE_RELEASE = 1${PACKAGE_RELEASE_SUFFIX}

PackageSummary = uHAL Library
PackageDescription = uHAL Library
PackageURL = https://ipbus.web.cern.ch/ipbus
Packager = Andrew Rose, Tom Williams


Library = cactus_uhal_uhal
LIBRARY_VER_ABI = $(PACKAGE_VER_MAJOR).$(PACKAGE_VER_MINOR)


IncludePaths = include  \
		${EXTERN_BOOST_INCLUDE_PREFIX} \
		${EXTERN_PUGIXML_INCLUDE_PREFIX} \
		${BUILD_HOME}/uhal/log/include \
		${BUILD_HOME}/uhal/grammars/include

LibraryPaths = ${EXTERN_BOOST_LIB_PREFIX} \
		${EXTERN_PUGIXML_LIB_PREFIX} \
		${BUILD_HOME}/uhal/log/lib \
		${BUILD_HOME}/uhal/grammars/lib

Libraries = pthread \
		\
		pugixml \
		\
		boost_filesystem \
		boost_regex \
		boost_system \
		${BOOST_THREAD_LIB} \
		boost_chrono \
		\
		cactus_uhal_grammars \
		cactus_uhal_log

#-DNO_PREEMPTIVE_DISPATCH
#-D THROW_ON_ADDRESS_SPACE_OVERLAP

# NOTE: Adding this flag is a dirty hack to make uhal/uhal compile on
# OS X. The fact that this is necessary really means the code is
# fishy.
ifeq ($(CACTUS_OS),osx)
CXXFLAGS += -Wno-c++11-extensions -fdelayed-template-parsing
endif



include $(BUILD_HOME)/uhal/config/mfRules.mk
include $(BUILD_HOME)/uhal/config/mfRPMRules.mk
include $(BUILD_HOME)/uhal/config/mfInstallRules.mk
