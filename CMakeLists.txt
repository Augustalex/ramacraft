cmake_minimum_required(VERSION 3.16)
project(RamaCraft VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(ramacraft ${SOURCES})
target_include_directories(ramacraft PRIVATE src)

# Dependencies
find_package(OpenGL REQUIRED)

if(WIN32)
    # Windows configuration
    find_package(SDL2 CONFIG QUIET)
    if(TARGET SDL2::SDL2)
        target_link_libraries(ramacraft PRIVATE SDL2::SDL2)
    else()
        find_package(SDL2 REQUIRED)
        target_include_directories(ramacraft PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(ramacraft PRIVATE ${SDL2_LIBRARIES})
    endif()

    target_link_libraries(ramacraft PRIVATE OpenGL::GL ws2_32)

    # Automatically copy SDL2.dll to binary folder if found
    if(TARGET SDL2::SDL2)
        add_custom_command(TARGET ramacraft POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:ramacraft>
        )
    endif()

elseif(APPLE)
    # macOS configuration
    find_package(SDL2 REQUIRED)
    target_include_directories(ramacraft PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(ramacraft PRIVATE ${SDL2_LIBRARIES} "-framework OpenGL")

else()
    # Linux configuration
    find_package(SDL2 REQUIRED)
    target_include_directories(ramacraft PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(ramacraft PRIVATE ${SDL2_LIBRARIES} OpenGL::GL pthread dl)
endif()
