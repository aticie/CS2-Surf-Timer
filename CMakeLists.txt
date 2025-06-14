cmake_minimum_required(VERSION 3.22)

project(cs2surf)

include("shared.cmake")

set(SURFTIMER_SRC "./src")

include_directories(
	${SURFTIMER_SRC}
	${SURFTIMER_SRC}/sdk/protobuf
	./vendor/libhv/include
	./vendor/libmem/include
	./vendor/libmodule/include
	./vendor/nlohmann
	./vendor/fmt/include
	./vendor/polyhook/include
	./vendor/zlib/include
)

if(WIN32)
	file(GLOB SURFTIMER_LIBS 
			"./vendor/libmem/lib/*.lib"
			"./vendor/libmodule/lib/*.lib"
			"./vendor/libhv/lib/*.lib"
			"./vendor/polyhook/lib/*.lib"
			"./vendor/zlib/lib/*.lib"
	)
else()
	file(GLOB SURFTIMER_LIBS 
			"./vendor/libmem/lib/*.a"
			"./vendor/libmodule/lib/*.a"
			"./vendor/libhv/lib/*.a"
			"./vendor/polyhook/lib/*.a"
			"./vendor/zlib/lib/*.a"
	)
endif()

file(GLOB_RECURSE SDK_SOURCES 
		"${SDK_CONVAR}"
		"${SDK_ROOT}/entity2/entityidentity.cpp" 
		"${SDK_ROOT}/entity2/entitysystem.cpp" 
		"${SDK_ROOT}/entity2/entitykeyvalues.cpp"
		"${SDK_ROOT}/tier1/keyvalues3.cpp"
)
file(GLOB_RECURSE PROTOBUF_SOURCES "${SURFTIMER_SRC}/sdk/protobuf/*.pb.cc" "${SURFTIMER_SRC}/sdk/protobuf/*.pb.h")
file(GLOB_RECURSE SURFTIMER_SOURCES "${SURFTIMER_SRC}/*.cpp" "${SURFTIMER_SRC}/*.h")

add_definitions(-DHV_STATICLIB -DHAVE_MEMOVERRIDE_NEW_DELETE -DFMT_HEADER_ONLY -DNOMINMAX)

add_library(${PROJECT_NAME} SHARED ${SDK_SOURCES} ${PROTOBUF_SOURCES} ${SURFTIMER_SOURCES})

if(WIN32)
	target_link_options(${PROJECT_NAME} PRIVATE "ntdll.lib")
else()
	find_package(OpenSSL REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${SURFTIMER_LIBS})

target_precompile_headers(${PROJECT_NAME} PRIVATE "${SURFTIMER_SRC}/pch.h")

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX ""
	INSTALL_RPATH "$ORIGIN/../lib"
	BUILD_WITH_INSTALL_RPATH TRUE
)

set(OUT_DIR "" CACHE PATH "SurfTimer binary directory for post-build file copy")

if (NOT ${OUT_DIR} STREQUAL "")
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			$<TARGET_FILE:${PROJECT_NAME}>
			${OUT_DIR}
	)
endif()

