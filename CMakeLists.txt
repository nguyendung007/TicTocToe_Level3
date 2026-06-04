cmake_minimum_required(VERSION 3.16)

# 1. Project name & C++ standard
project(TicTacToe-LV3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Recursively gather sources from src/ — TRỪ recursion_combinator.cpp
#    (file demo Y combinator có main() riêng, không link vào game).
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*recursion_combinator\\.cpp$")

# 3. SDL — chỉ kích hoạt nếu sinh viên muốn giữ GUI từ level 2.
#    Mặc định TẮT để build sạch trên máy không có SDL.
option(USE_SDL "Build with SDL2 GUI" OFF)

if(USE_SDL)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
endif()

# 4. Executable target
add_executable(game ${SOURCES})

# 5. Include paths
target_include_directories(game PRIVATE
    src
    $<$<BOOL:${USE_SDL}>:${SDL2_INCLUDE_DIRS}>
)

# 6. Link libraries
if(USE_SDL)
    if(WIN32)
        target_link_libraries(game PRIVATE
            mingw32
            SDL2::SDL2main
            SDL2::SDL2
            SDL2_ttf::SDL2_ttf
        )
    else()
        target_link_libraries(game PRIVATE
            SDL2::SDL2
            SDL2_ttf::SDL2_ttf
        )
    endif()
endif()

# 7. Một số cờ cần cho mức 3 nhánh B (parallel STL):
#    - GCC/Clang: cần TBB ở runtime cho std::execution::par.
#    - MSVC:     cần /EHsc.
if(MSVC)
    target_compile_options(game PRIVATE /W4 /O2 /EHsc)
else()
    target_compile_options(game PRIVATE -Wall -Wextra -O2)
    # Thread support (cho std::async / std::thread).
    find_package(Threads REQUIRED)
    target_link_libraries(game PRIVATE Threads::Threads)

    # Nếu sinh viên dùng std::execution::par, có thể cần TBB:
    #   sudo apt install libtbb-dev   (Ubuntu/Debian)
    # Bỏ comment 2 dòng sau nếu cần.
    # find_package(TBB REQUIRED)
    # target_link_libraries(game PRIVATE TBB::tbb)
endif()

# 8. Output binary tại thư mục source (giống level 2)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# 9. Copy assets/ vào cạnh binary nếu thư mục tồn tại (chỉ cần cho SDL)
if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:game>/assets
    )
endif()

# ──────────────────────────────────────────────────────────────────────
# (TUỲ CHỌN) Build target riêng cho demo Y combinator (mức 3 nhánh A)
#   cmake --build . --target y_demo
# ──────────────────────────────────────────────────────────────────────
add_executable(y_demo EXCLUDE_FROM_ALL src/core/recursion_combinator.cpp)
target_compile_definitions(y_demo PRIVATE RUN_Y_DEMO)
