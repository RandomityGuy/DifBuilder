cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
include (CheckIncludeFiles)
project (DifBuilder C CXX)

add_subdirectory(3rdparty/Dif)

# Allow C++11 features
if (UNIX)
list (APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
else()
list (APPEND CMAKE_CXX_FLAGS "/MP /FS ${CMAKE_CXX_FLAGS}")
endif()

# Enable Static Linking the C++ ABI directly into the executables and libraries
if (MSVC)
	#---BEGIN COPY FROM BULLET CMAKE---
	#We statically link to reduce dependancies
	FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO )
		IF(${flag_var} MATCHES "/MD")
			STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		ENDIF(${flag_var} MATCHES "/MD")
		IF(${flag_var} MATCHES "/MDd")
			STRING(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
		ENDIF(${flag_var} MATCHES "/MDd")
	ENDFOREACH(flag_var)
	#---END COPY FROM BULLET CMAKE---
endif()

# http://stackoverflow.com/a/7750816
# Basically just force the output dir to where we give
macro(target_output_directory targetname directory)
	set_target_properties( ${targetname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${directory} )
	set_target_properties( ${targetname} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${directory} )
	set_target_properties( ${targetname} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${directory} )
	# Second, for multi-config builds (e.g. msvc)
	foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
		set_target_properties( ${targetname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${directory} )
		set_target_properties( ${targetname} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${directory} )
		set_target_properties( ${targetname} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${directory} )
	endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endmacro()

SET (DIFBUILDER_SRC
	src/DIFBuilder/DIFBuilder.cpp
	src/DIFBuilder/BSPLib.cpp
	include/DIFBuilder/DIFBuilder.hpp
	include/DIFBuilder/BSPLib.h
)

# Create the library
add_library(DifBuilder STATIC ${DIFBUILDER_SRC})
include_directories(include)
include_directories(3rdparty)
include_directories(3rdparty/Dif)
include_directories(3rdparty/Dif/3rdparty/glm)
include_directories(3rdparty/Dif/include)

