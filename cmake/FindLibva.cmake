# NOTE: I, Thomas Erbesdobler <t.erbesdobler@team103.com>, created 
# this file as addition to FreeRDP

# - Try to find Libva for using intel's VAAPI
# ... just like OpenH264 is handled ...
# Once done this will define
#
#  LIBVA_ROOT - A list of search hints
#
#  LIBVA_FOUND - system has Libva
#  LIBVA_INCLUDE_DIR - the Libva include directory
#  LIBVA_LIBRARIES - Libva library

if (LIBVA_INCLUDE_DIR AND LIBVA_LIBRARY)
	set(LIBVA_FIND_QUIETLY TRUE)
endif (LIBVA_INCLUDE_DIR AND LIBVA_LIBRARY)

find_path(LIBVA_INCLUDE_DIR NAMES va/va.h va/va_x11.h
	PATH_SUFFIXES include
	HINTS ${LIBVA_ROOT})

find_library(LIBVA_LIBRARY NAMES va
	 PATH_SUFFIXES lib
	 HINTS ${LIBVA_ROOT})

find_library (LIBVA_X11_LIBRARY NAMES va-x11
	 PATH_SUFFIXES lib
	 HINTS ${LIBVA_ROOT})

if (LIBVA_LIBRARY AND LIBVA_X11_LIBRARY)
	set (LIBVA_LIBS_FOUND TRUE)
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Libva DEFAULT_MSG LIBVA_LIBS_FOUND LIBVA_INCLUDE_DIR)

if (LIBVA_INCLUDE_DIR AND LIBVA_LIBS_FOUND)
	set(LIBVA_FOUND TRUE)
	set(LIBVA_LIBRARIES ${LIBVA_LIBRARY} ${LIBVA_X11_LIBRARY})
endif (LIBVA_INCLUDE_DIR AND LIBVA_LIBS_FOUND)

if (LIBVA_FOUND)
	if (NOT LIBVA_FIND_QUIETLY)
		message(STATUS "Found Libva: ${LIBVA_LIBRARIES}")
	endif (NOT LIBVA_FIND_QUIETLY)
else (LIBVA_FOUND)
	if (LIBVA_FIND_REQUIRED)
		message(FATAL_ERROR "Libva was not found")
	endif(LIBVA_FIND_REQUIRED)
endif (LIBVA_FOUND)

mark_as_advanced(LIBVA_INCLUDE_DIR LIBVA_LIBRARY)

