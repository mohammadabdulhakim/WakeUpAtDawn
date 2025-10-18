cmake_minimum_required(VERSION 3.19)
project(untitled LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# 🔹 إعداد الأيقونة (Windows فقط)
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/icons/app_icon.rc")

if(WIN32)
    qt_add_executable(untitled
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        resources.qrc
        stb_image.cpp
        Filters.h
    )
else()
    qt_add_executable(untitled
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        resources.qrc
        stb_image.cpp
        Filters.h
    )
endif()

# ✅ نسخ ملف الـ QSS بعد الـ Build
configure_file(${CMAKE_SOURCE_DIR}/styles.qss
               ${CMAKE_BINARY_DIR}/styles.qss COPYONLY)

# ✅ نسخ مجلد الأيقونات بالكامل (مرة واحدة بعد الـ build)
add_custom_command(TARGET untitled POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/icons"
            "${CMAKE_BINARY_DIR}/icons"
    COMMENT "🔁 Copying icons to build folder..."
)

# 🔹 ربط مكتبات Qt
target_link_libraries(untitled
    PRIVATE Qt::Core Qt::Widgets
)

# 🔹 إعدادات التثبيت
include(GNUInstallDirs)
install(TARGETS untitled
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
