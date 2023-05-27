cmake_minimum_required(VERSION 3.4)
project(Tanktask)

add_subdirectory(pybind11)

# Set the source directory containing your source files
set(SOURCE_DIR "TankTrouble")

# Use the file(GLOB ...) command to get a list of all source files in the directory
file(GLOB SOURCES "${SOURCE_DIR}/*.cpp")

# Add the source files to your Python extension module
pybind11_add_module(Tanktask ${SOURCES})

target_include_directories(Tanktask PRIVATE Tanktrouble)
target_include_directories(Tanktask PRIVATE extern/Box2D/include/box2d)
target_include_directories(Tanktask PRIVATE extern/SDL2/SDL2-2.26.5/include)
target_include_directories(Tanktask PRIVATE extern/SDL2/SDL2_image-2.6.3/include)

target_link_directories(Tanktask PRIVATE ${CMAKE_SOURCE_DIR}/extern/Box2D/lib)
target_link_directories(Tanktask PRIVATE ${CMAKE_SOURCE_DIR}/extern/SDL2/SDL2-2.26.5/lib/x64)
target_link_directories(Tanktask PRIVATE ${CMAKE_SOURCE_DIR}/extern/SDL2/SDL2_image-2.6.3/lib/x64)

target_link_libraries(Tanktask PRIVATE box2d.lib SDL2.lib SDL2main.lib SDL2_image.lib)
