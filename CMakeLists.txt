# Drogon требует CMake версии 3.16+
cmake_minimum_required(VERSION 3.16)
project(DrogonTaskManager)

# Drogon требует C++17, без указания стандарта C++17 CMake выдаст ошибку
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем пакет Drogon, REQUIRED означает, что если пакет не найден, CMake выдаст ошибку
find_package(Drogon REQUIRED)

# создаем исполняемый файл drogontaskmanager из исходного файла main.cc
add_executable(drogontaskmanager 
    main.cc
)

# Добавляем корневую папку проектв в список директорий для поиска заголовочных файлов.
# PRIVATE означает, что эта директория будет использоваться только для сборки этого исполняемого файла 
target_include_directories(drogontaskmanager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Линковка с Drogon
target_link_libraries(drogontaskmanager PRIVATE Drogon::Drogon)