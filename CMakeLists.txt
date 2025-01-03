# CMakeList.txt : CMake project for OpticalFlow, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("OpticalFlow")

include(E:/Electro/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(PNG REQUIRED)
find_package(FLTK REQUIRED CONFIG)

find_path(PNGPP_INCLUDE_DIRS "png++/color.hpp")

include_directories(${PNG_INCLUDE_DIR})
include_directories(${PNGPP_INCLUDE_DIRS})


# Add source to this project's executable.
add_executable (OpticalFlow 
	"Frame.cpp"
	"OpticalFlow.cpp" 
	"OpticalFlow.h"

	
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET OpticalFlow PROPERTY CXX_STANDARD 20)
endif()

link_directories(${VCPKG_INSTALLED_DIR}/x64-windows/lib)
target_link_libraries(OpticalFlow PRIVATE fltk fltk_gl fltk_forms fltk_images)
target_link_libraries(OpticalFlow PRIVATE PNG::PNG)

# TODO: Add tests and install targets if needed.
