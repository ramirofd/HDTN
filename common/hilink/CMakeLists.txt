add_library(hilink_lib
        src/HilinkTcpBundleSource.cpp
        src/HilinkTcpBundleSink.cpp
)
GENERATE_EXPORT_HEADER(hilink_lib)
get_target_property(target_type hilink_lib TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
        set_property(TARGET hilink_lib PROPERTY CXX_VISIBILITY_PRESET hidden)
        set_property(TARGET hilink_lib PROPERTY VISIBILITY_INLINES_HIDDEN ON)
endif()
set(MY_PUBLIC_HEADERS
        include/HilinkTcpBundleSource.h
        include/HilinkTcpBundleSink.h
        ${CMAKE_CURRENT_BINARY_DIR}/hilink_lib_export.h
)
set_target_properties(hilink_lib PROPERTIES PUBLIC_HEADER "${MY_PUBLIC_HEADERS}")
target_link_libraries(hilink_lib
        PUBLIC
                stcp_lib
                log_lib
)
target_include_directories(hilink_lib
        PUBLIC
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
install(TARGETS hilink_lib
        EXPORT hilink_lib-targets
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
add_hdtn_package_export(hilink_lib HilinkLib)
