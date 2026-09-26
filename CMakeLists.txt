cmake_minimum_required(VERSION 3.16)
project(libstructs C)

# --- User-adjustable settings -----------------------------------------
# These show up as normal CMake cache variables. You can override any
# of them at configure time, for example:
#   cmake -B build -DLIBSTRUCTS_LOAD_FACTOR=0.6
#
# LIBSTRUCTS_TARGET_MEMORY_KB is left blank by default, which means
# "figure it out automatically." Set it by hand when you are cross
# compiling for a target that is not the machine you are building on,
# since CMake can only detect the RAM of the machine actually running the build.
option(LIBSTRUCTS_AUTO_DETECT_MEMORY "Detect host RAM and choose defaults from it" ON)
set(LIBSTRUCTS_TARGET_MEMORY_KB "" CACHE STRING "Manually set the memory budget in KB. Leave blank to auto-detect.")
set(LIBSTRUCTS_LOAD_FACTOR "0.75" CACHE STRING "Hash map resize threshold")
set(LIBSTRUCTS_INITIAL_CAPACITY "4" CACHE STRING "Starting capacity for vector and hash map")

# --- Memory detection ----------------------------------------------------
# cmake_host_system_information can ask the operating system how much
# RAM is installed. This only runs if the person did not already set
# LIBSTRUCTS_TARGET_MEMORY_KB by hand above.
if(LIBSTRUCTS_TARGET_MEMORY_KB STREQUAL "")
  if(LIBSTRUCTS_AUTO_DETECT_MEMORY)
    cmake_host_system_information(RESULT HOST_RAM_MB QUERY TOTAL_PHYSICAL_MEMORY)
    math(EXPR LIBSTRUCTS_TARGET_MEMORY_KB "${HOST_RAM_MB} * 1024")
    message(STATUS "libstructs: detected ${HOST_RAM_MB} MB of RAM on this machine")
  else()
    set(LIBSTRUCTS_TARGET_MEMORY_KB "1048576") # fall back to a 1 GB assumption
    message(STATUS "libstructs: auto-detect disabled, assuming 1 GB budget")
  endif()
endif()

# --- Pick tighter defaults on constrained targets ------------------------
# Below 64 MB, prefer a smaller starting capacity and a stricter load
# factor, so structures grow more conservatively. This threshold and
# the numbers below are a starting point, adjust them once you have
# real numbers from testing on your target.
if(LIBSTRUCTS_TARGET_MEMORY_KB LESS 65536)
  set(LIBSTRUCTS_INITIAL_CAPACITY "2")
  set(LIBSTRUCTS_LOAD_FACTOR "0.6")
  message(STATUS "libstructs: memory budget is small, using conservative defaults")
endif()

# --- Write the generated config header -----------------------------------
# This takes cmake/config_in.h, replaces the @...@ placeholders with the
# values computed above, and writes the result into the build directory.
# generated/libstructs_config.h is what your structure code will include.
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/config_in.h
    ${CMAKE_SOURCE_DIR}/generated/libstructs_config.h
)

# --- Code generation -------------------------------------
# Runs gen.py before the build, so generated/*.c and generated/*.h exist
# before CMake tries to compile anything.
add_custom_target(generate
    COMMAND python3 ${CMAKE_SOURCE_DIR}/gen.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating typed structures from templates"
)

# --- Library target (fill in once structures exist) -----------------------
# file(GLOB ...) picks up every generated .c file automatically. GLOB is
# usually discouraged in CMake because it does not notice new files
# without a manual reconfigure, but for a generated/ directory that is
# fine, since a new type means running gen.py again anyway, which you
# will do by hand.
file(GLOB LIBSTRUCTS_SOURCES ${CMAKE_SOURCE_DIR}/generated/*.c ${CMAKE_SOURCE_DIR}/src/*.c)

if(LIBSTRUCTS_SOURCES)
  add_library(libstructs STATIC ${LIBSTRUCTS_SOURCES})
  add_dependencies(libstructs generate)
  target_include_directories(libstructs PUBLIC
        ${CMAKE_SOURCE_DIR}/generated
        ${CMAKE_SOURCE_DIR}/src
    )
endif()

# --- Tests ---------------------------
enable_testing()
file(GLOB LIBSTRUCTS_TEST_SOURCES ${CMAKE_SOURCE_DIR}/tests/test_*.c)
foreach(test_src ${LIBSTRUCTS_TEST_SOURCES})
  get_filename_component(test_name ${test_src} NAME_WE)
  add_executable(${test_name} ${test_src})
  target_link_libraries(${test_name} PRIVATE libstructs)
  target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/tests)
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# --- Freestanding safety check (kernel-portability check) -----------------
# This must be a full link, not just a compile. Compiling with -c against
# -nostdlib never actually fails, because malloc/free/printf are only
# declarations at that point and get resolved at link time. Only linking
# (with no libc available to satisfy them) turns a hidden libc call into
# an actual error: "undefined reference to `malloc'". -nostartfiles just
# silences a harmless "cannot find entry symbol _start" warning that has
# nothing to do with what we're checking for.
#
# src/allocator.c is deliberately excluded: it is the one file allowed to
# call malloc/realloc/free directly, since it exists to provide the
# libc-backed Allocator for hosted builds. Only generated/*.c (the actual
# structure code) is checked here.
if(LIBSTRUCTS_SOURCES)
  add_custom_target(freestanding-check
        COMMAND bash -c "for f in ${CMAKE_SOURCE_DIR}/generated/*.c; do \
            echo \"checking $$f\"; \
            ${CMAKE_C_COMPILER} -ffreestanding -nostdlib -nostartfiles \"$$f\" \
                -I${CMAKE_SOURCE_DIR}/generated -I${CMAKE_SOURCE_DIR}/src \
                -o /dev/null || exit 1; \
        done"
        DEPENDS generate
        COMMENT "Verifying generated code has no hidden libc dependency (link-time check)"
    )
endif()
