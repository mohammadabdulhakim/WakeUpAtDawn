cmake_minimum_required(VERSION 3.19)
project(WakeUpAtDawn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# --------------------------------------------------
# 🔹 إعداد الأيقونة (Windows فقط)
# --------------------------------------------------
if(WIN32)
    # ملف .rc لازم يشير لأيقونة .ico
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/icons/app_icon.rc")
endif()

# --------------------------------------------------
# 🔹 الملفات التنفيذية
# --------------------------------------------------
qt_add_executable(WakeUpAtDawn
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resources.qrc
    stb_image.cpp
    Filters.h
    ${APP_ICON_RESOURCE_WINDOWS} # ← مهم جدًا
)

# 🟢 إلغاء ظهور نافذة الـ CMD
set_target_properties(WakeUpAtDawn PROPERTIES WIN32_EXECUTABLE TRUE)


# --------------------------------------------------
# ✅ نسخ ملف QSS
# --------------------------------------------------
configure_file(${CMAKE_SOURCE_DIR}/styles.qss
               ${CMAKE_BINARY_DIR}/styles.qss COPYONLY)

# ✅ نسخ مجلد الأيقونات بعد البناء
add_custom_command(TARGET WakeUpAtDawn POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/icons"
            "${CMAKE_BINARY_DIR}/icons"
    COMMENT "🔁 Copying icons to build folder..."
)

# --------------------------------------------------
# 🔹 ربط مكتبات Qt
# --------------------------------------------------
target_link_libraries(WakeUpAtDawn
    PRIVATE Qt::Core Qt::Widgets
)

# --------------------------------------------------
# 🔹 إعدادات التثبيت
# --------------------------------------------------
include(GNUInstallDirs)
install(TARGETS WakeUpAtDawn
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
