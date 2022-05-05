cmake_minimum_required(VERSION 3.3.1)
include_guard()

function(package_create)

    include(GNUInstallDirs)
    message("CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")

    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/pkg CACHE PATH "" FORCE)
    endif()
    message("CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

    set(INSTALL_PACKAGEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
    message("INSTALL_PACKAGEDIR = ${INSTALL_PACKAGEDIR}")

    install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        )

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${INSTALL_PACKAGEDIR}
        )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "\$\{_IMPORT_PREFIX\}\/include"
    )

#    set_target_properties(${PROJECT_NAME} PROPERTIES
#        "INTERFACE_INCLUDE_DIRECTORIES" "${CMAKE_INSTALL_PREFIX}/include"
#        )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config.cmake
        INSTALL_DESTINATION ${INSTALL_PACKAGEDIR}
        )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake
        VERSION ${pod_VERSION}
        COMPATIBILITY AnyNewerVersion
        )
    install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake
        DESTINATION ${INSTALL_PACKAGEDIR}
        )
endfunction()
