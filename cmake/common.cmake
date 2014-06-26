# Determine the effective build directory

SET(PROJECT_BUILD_DIR ${CMAKE_BINARY_DIR})

IF(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
    SET(PROJECT_BUILD_DIR ${PROJECT_BUILD_DIR}/${CMAKE_CFG_INTDIR})
ENDIF()

#===============================================================================

MACRO(INITIALISE_PROJECT)
#    SET(CMAKE_VERBOSE_MAKEFILE ON)
    SET(CMAKE_INCLUDE_CURRENT_DIR ON)

    # Make sure that we are building on a supported architecture
    # Note: normally, we would check the value of CMAKE_SIZEOF_VOID_P, but in
    #       some cases it may not be set (e.g. when generating an Xcode project
    #       file), so we determine and retrieve that value ourselves...

    TRY_RUN(ARCHITECTURE_RUN ARCHITECTURE_COMPILE
            ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/cmake/architecture.c
            RUN_OUTPUT_VARIABLE ARCHITECTURE)

    IF(NOT ${ARCHITECTURE} EQUAL 32 AND NOT ${ARCHITECTURE} EQUAL 64)
        MESSAGE(FATAL_ERROR "OpenCOR can only be built in 32-bit or 64-bit mode...")
    ENDIF()

    # By default, we are building a release version of OpenCOR, unless we are
    # explicitly asked for a debug version

    IF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        MESSAGE("Building a ${ARCHITECTURE}-bit debug version...")

        SET(RELEASE_MODE OFF)
    ELSE()
        MESSAGE("Building a ${ARCHITECTURE}-bit release version...")

        SET(RELEASE_MODE ON)
    ENDIF()

    # Required packages

    IF(APPLE)
        SET(MAC_EXTRAS MacExtras)
    ELSE()
        SET(MAC_EXTRAS)
    ENDIF()

    SET(REQUIRED_QT_MODULES
        Concurrent
        Help
        ${MAC_EXTRAS}
        Network
        PrintSupport
        Svg
        UiTools
        WebKitWidgets
        Widgets
        Xml
        XmlPatterns
    )

    FOREACH(REQUIRED_QT_MODULE ${REQUIRED_QT_MODULES})
        FIND_PACKAGE(Qt5${REQUIRED_QT_MODULE} REQUIRED)
    ENDFOREACH()

    IF(ENABLE_TESTS)
        FIND_PACKAGE(Qt5Test REQUIRED)
    ENDIF()

    # Keep track of some information about Qt

    SET(QT_BINARY_DIR ${_qt5Widgets_install_prefix}/bin)
    SET(QT_LIBRARY_DIR ${_qt5Widgets_install_prefix}/lib)
    SET(QT_PLUGINS_DIR ${_qt5Widgets_install_prefix}/plugins)
    SET(QT_VERSION_MAJOR ${Qt5Widgets_VERSION_MAJOR})
    SET(QT_VERSION_MINOR ${Qt5Widgets_VERSION_MINOR})
    SET(QT_VERSION_PATCH ${Qt5Widgets_VERSION_PATCH})

    # Some general build settings
    # Note: we need to use C++11, so that we can define strings as static const.
    #       Now, it happens that MSVC enables C++11 support by default, so we
    #       just need to enable it on Linux and OS X. In that context, and for
    #       backward compatilibity with versions of gcc older than 4.7, we do
    #       this by using -std=c++0x rather than -std=c++11...

    IF(WIN32)
        STRING(REPLACE "/W3" "/W3 /WX" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        # Note: MSVC has a /Wall flag, but it results in MSVC being very
        #       pedantic, so instead we use what MSVC recommends for production
        #       code which is /W3 and which is also what CMake uses by
        #       default...

        SET(LINK_FLAGS_PROPERTIES "/STACK:10000000 /MACHINE:X86")
    ELSE()
        SET(CMAKE_CXX_FLAGS "-Wall -W -Werror -std=c++0x")

        IF(APPLE)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            SET(LINK_FLAGS_PROPERTIES "-stdlib=libc++")
        ELSE()
            SET(LINK_FLAGS_PROPERTIES)
        ENDIF()
    ENDIF()

    # Some build settings that depend on whether we want a debug or release
    # version of OpenCOR

    IF(RELEASE_MODE)
        # Default compiler and linker settings
        # Note: OpenCOR is built using gcc on Linux. However, in gcc, the -O3
        #       option comes with a warning: "Under some circumstances where
        #       these optimizations are not favorable, this option might
        #       actually make a program slower." This is the reason we use -O2.
        #       On the other hand, OpenCOR is built using Clang on OS X, hence
        #       we use -O3 in that case...

        IF(WIN32)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DNDEBUG /MD /O2 /Ob2")
        ELSE()
            IF(APPLE)
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
            ELSE()
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
            ENDIF()

            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
        ENDIF()

        IF(NOT WIN32 AND NOT APPLE)
            SET(LINK_FLAGS_PROPERTIES "${LINK_FLAGS_PROPERTIES} -Wl,-s")
            # Note #1: -Wl,-s strips all the symbols, thus reducing the final
            #          size of OpenCOR or one its shared libraries...
            # Note #2: the above linking option has become obsolete on OS X,
            #          so...
        ENDIF()

        # Make sure that debugging is off in Qt

        ADD_DEFINITIONS(-DQT_NO_DEBUG)
    ELSE()
        # Default compiler and linker settings

        IF(WIN32)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_DEBUG /MDd /Zi /Ob0 /Od /RTC1")
            SET(LINK_FLAGS_PROPERTIES "${LINK_FLAGS_PROPERTIES} /DEBUG")
        ELSE()
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
        ENDIF()

        # Make sure that debugging is on in Qt

        ADD_DEFINITIONS(-DQT_DEBUG)
    ENDIF()

    # Ask for Unicode to be used

    ADD_DEFINITIONS(-DUNICODE)

    IF(WIN32)
        ADD_DEFINITIONS(-D_UNICODE)
    ENDIF()

    # Sample plugins support, if requested

    IF(ENABLE_SAMPLES)
        ADD_DEFINITIONS(-DENABLE_SAMPLES)
    ENDIF()

    # On OS X, use the oldest SDK available, as long as it is for Mac OS X 10.7
    # or later (i.e. a version that supports C++11)

    IF(APPLE)
        EXECUTE_PROCESS(COMMAND xcodebuild -showsdks
                        COMMAND grep --colour=never macosx
                        COMMAND cut -f2
                        COMMAND sed "s/ *$//"
                        OUTPUT_VARIABLE OS_X_VERSIONS
                        ERROR_QUIET)
        EXECUTE_PROCESS(COMMAND echo ${OS_X_VERSIONS}
                        COMMAND rev
                        COMMAND cut -d " " -f1
                        COMMAND rev
                        COMMAND tr "\n" ";"
                        OUTPUT_VARIABLE OS_X_VERSION_NUMBERS)

        SET(VALID_OS_X_VERSION_NUMBER)
        SET(INVALID_OS_X_VERSION_NUMBER)

        FOREACH(OS_X_VERSION_NUMBER ${OS_X_VERSION_NUMBERS})
            IF(${OS_X_VERSION_NUMBER} VERSION_LESS 10.7)
                SET(INVALID_OS_X_VERSION_NUMBER ${OS_X_VERSION_NUMBER})
            ELSE()
                IF("${VALID_OS_X_VERSION_NUMBER}" STREQUAL "")
                    SET(VALID_OS_X_VERSION_NUMBER ${OS_X_VERSION_NUMBER})
                ENDIF()
            ENDIF()
        ENDFOREACH()

        IF("${VALID_OS_X_VERSION_NUMBER}" STREQUAL "")
            IF("${INVALID_OS_X_VERSION_NUMBER}" STREQUAL "")
                MESSAGE(FATAL_ERROR "No SDK could be found...")
            ELSE()
                EXECUTE_PROCESS(COMMAND echo ${OS_X_VERSIONS}
                                COMMAND grep --colour=never "${INVALID_OS_X_VERSION_NUMBER}"
                                OUTPUT_VARIABLE INVALID_OS_X_VERSION
                                OUTPUT_STRIP_TRAILING_WHITESPACE)

                MESSAGE(FATAL_ERROR "The ${INVALID_OS_X_VERSION} SDK was found, but the Mac OS X 10.7 SDK or later is needed...")
            ENDIF()
        ENDIF()

        EXECUTE_PROCESS(COMMAND echo ${OS_X_VERSIONS}
                        COMMAND grep --colour=never "${VALID_OS_X_VERSION_NUMBER}"
                        OUTPUT_VARIABLE VALID_OS_X_VERSION
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        EXECUTE_PROCESS(COMMAND xcodebuild -version -sdk macosx${VALID_OS_X_VERSION_NUMBER} Path
                        OUTPUT_VARIABLE VALID_SDK_PATH
                        OUTPUT_STRIP_TRAILING_WHITESPACE)

        MESSAGE("Building for ${VALID_OS_X_VERSION} and later...")

        SET(CMAKE_OSX_DEPLOYMENT_TARGET ${VALID_OS_X_VERSION_NUMBER})
        SET(CMAKE_OSX_SYSROOT ${VALID_SDK_PATH})
    ENDIF()

    # Location of our plugins so that we don't have to deploy OpenCOR on
    # Windows and Linux before being able to test it

    IF(APPLE)
        SET(DEST_PLUGINS_DIR ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/PlugIns/${CMAKE_PROJECT_NAME})
    ELSE()
        SET(DEST_PLUGINS_DIR ${PROJECT_BUILD_DIR}/plugins/${CMAKE_PROJECT_NAME})
    ENDIF()

    # Default location of external dependencies

    IF(WIN32)
        SET(DISTRIB_DIR windows/x86)
    ELSEIF(APPLE)
        SET(DISTRIB_DIR osx)
    ELSE()
        IF(${ARCHITECTURE} EQUAL 32)
            SET(DISTRIB_DIR linux/x86)
        ELSE()
            SET(DISTRIB_DIR linux/x64)
        ENDIF()
    ENDIF()

    IF(WIN32)
        IF(RELEASE_MODE)
            SET(DISTRIB_BINARY_DIR ${DISTRIB_DIR}/release)
        ELSE()
            SET(DISTRIB_BINARY_DIR ${DISTRIB_DIR}/debug)
        ENDIF()
    ELSE()
        SET(DISTRIB_BINARY_DIR ${DISTRIB_DIR})
    ENDIF()

    # Set the RPATH information on Linux

    IF(NOT WIN32 AND NOT APPLE)
        SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/../plugins/${PROJECT_NAME}")
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(UPDATE_LANGUAGE_FILES TARGET_NAME)
    # Update the translation (.ts) files (if they exist) and generate the
    # language (.qm) files, which will later on be embedded in the project
    # Note: this requires SOURCES, HEADERS_MOC and UIS to be defined for the
    #       current CMake project, even if that means that these variables are
    #       to be empty (the case with some plugins for example). Indeed, since
    #       otherwise the value of these variables, as defined in a previous
    #       project, may be used, so...

    SET(LANGUAGE_FILES
        ${TARGET_NAME}_fr
    )

    FOREACH(LANGUAGE_FILE ${LANGUAGE_FILES})
        SET(TS_FILE i18n/${LANGUAGE_FILE}.ts)

        IF(EXISTS ${PROJECT_SOURCE_DIR}/${TS_FILE})
            EXECUTE_PROCESS(COMMAND ${QT_BINARY_DIR}/lupdate -no-obsolete
                                                             ${SOURCES} ${HEADERS_MOC} ${UIS}
                                                         -ts ${TS_FILE}
                            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
            EXECUTE_PROCESS(COMMAND ${QT_BINARY_DIR}/lrelease ${PROJECT_SOURCE_DIR}/${TS_FILE}
                                                          -qm ${CMAKE_BINARY_DIR}/${LANGUAGE_FILE}.qm)
        ENDIF()
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(ADD_PLUGIN PLUGIN_NAME)
    # Various initialisations

    SET(PLUGIN_NAME ${PLUGIN_NAME})

    SET(SOURCES)
    SET(HEADERS_MOC)
    SET(UIS)
    SET(INCLUDE_DIRS)
    SET(DEFINITIONS)
    SET(PLUGINS)
    SET(PLUGIN_BINARIES)
    SET(QT_MODULES)
    SET(QT_LIBRARIES)
    SET(EXTERNAL_BINARIES_DIR)
    SET(EXTERNAL_BINARIES)
    SET(TESTS)

    # Analyse the extra parameters

    SET(TYPE_OF_PARAMETER 0)

    FOREACH(PARAMETER ${ARGN})
        IF("${PARAMETER}" STREQUAL "THIRD_PARTY")
            # We are dealing with a third-party plugin, so disable all warnings
            # since it may generate some and this is not something we have
            # control over
            # Note: for some reasons, MSVC eventually uses /W1, so we can't
            #       replace /W3 /WX with /w since this would conflict with
            #       /W1 and generate a warning, so...

            IF(WIN32)
                STRING(REPLACE "/W3 /WX" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
            ELSE()
                STRING(REPLACE "-Wall -W -Werror" "-w" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
            ENDIF()

            # Add a definition in case of compilation from within Qt Creator
            # using MSVC and JOM since the latter overrides some of our settings

            IF(WIN32)
                ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
            ENDIF()
        ELSEIF("${PARAMETER}" STREQUAL "SOURCES")
            SET(TYPE_OF_PARAMETER 1)
        ELSEIF("${PARAMETER}" STREQUAL "HEADERS_MOC")
            SET(TYPE_OF_PARAMETER 2)
        ELSEIF("${PARAMETER}" STREQUAL "UIS")
            SET(TYPE_OF_PARAMETER 3)
        ELSEIF("${PARAMETER}" STREQUAL "INCLUDE_DIRS")
            SET(TYPE_OF_PARAMETER 4)
        ELSEIF("${PARAMETER}" STREQUAL "DEFINITIONS")
            SET(TYPE_OF_PARAMETER 5)
        ELSEIF("${PARAMETER}" STREQUAL "PLUGINS")
            SET(TYPE_OF_PARAMETER 6)
        ELSEIF("${PARAMETER}" STREQUAL "PLUGIN_BINARIES")
            SET(TYPE_OF_PARAMETER 7)
        ELSEIF("${PARAMETER}" STREQUAL "QT_MODULES")
            SET(TYPE_OF_PARAMETER 8)
        ELSEIF("${PARAMETER}" STREQUAL "QT_LIBRARIES")
            SET(TYPE_OF_PARAMETER 9)
        ELSEIF("${PARAMETER}" STREQUAL "EXTERNAL_BINARIES_DIR")
            SET(TYPE_OF_PARAMETER 10)
        ELSEIF("${PARAMETER}" STREQUAL "EXTERNAL_BINARIES")
            SET(TYPE_OF_PARAMETER 11)
        ELSEIF("${PARAMETER}" STREQUAL "TESTS")
            SET(TYPE_OF_PARAMETER 12)
        ELSE()
            # Not one of the headers, so add the parameter to the corresponding
            # set

            IF(${TYPE_OF_PARAMETER} EQUAL 1)
                LIST(APPEND SOURCES ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 2)
                LIST(APPEND HEADERS_MOC ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 3)
                LIST(APPEND UIS ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 4)
                LIST(APPEND INCLUDE_DIRS ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 5)
                LIST(APPEND DEFINITIONS ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 6)
                LIST(APPEND PLUGINS ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 7)
                LIST(APPEND PLUGIN_BINARIES ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 8)
                LIST(APPEND QT_MODULES ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 9)
                LIST(APPEND QT_LIBRARIES ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 10)
                SET(EXTERNAL_BINARIES_DIR ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 11)
                LIST(APPEND EXTERNAL_BINARIES ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 12)
                LIST(APPEND TESTS ${PARAMETER})
            ENDIF()
        ENDIF()
    ENDFOREACH()

    # Various include directories

    SET(PLUGIN_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)

    INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

    # Resource file, if any

    SET(QRC_FILE res/${PLUGIN_NAME}.qrc)

    IF(EXISTS ${PROJECT_SOURCE_DIR}/${QRC_FILE})
        SET(RESOURCES ${QRC_FILE})
    ELSE()
        SET(RESOURCES)
    ENDIF()

    # Update the translation (.ts) files and generate the language (.qm) files,
    # which will later on be embedded in the plugin

    IF(NOT "${RESOURCES}" STREQUAL "")
        UPDATE_LANGUAGE_FILES(${PLUGIN_NAME})
    ENDIF()

    # Definition to make sure that the plugin can be used by other plugins

    ADD_DEFINITIONS(-D${PLUGIN_NAME}_PLUGIN)

    # Some plugin-specific definitions

    FOREACH(DEFINITION ${DEFINITIONS})
        ADD_DEFINITIONS(-D${DEFINITION})
    ENDFOREACH()

    # Generate and add the different files needed by the plugin

    IF("${HEADERS_MOC}" STREQUAL "")
        SET(SOURCES_MOC)
    ELSE()
        QT5_WRAP_CPP(SOURCES_MOC ${HEADERS_MOC})
    ENDIF()

    IF("${UIS}" STREQUAL "")
        SET(SOURCES_UIS)
    ELSE()
        QT5_WRAP_UI(SOURCES_UIS ${UIS})
    ENDIF()

    IF("${RESOURCES}" STREQUAL "")
        SET(SOURCES_RCS)
    ELSE()
        QT5_ADD_RESOURCES(SOURCES_RCS ${RESOURCES})
    ENDIF()

    ADD_LIBRARY(${PROJECT_NAME} SHARED
        ${SOURCES}
        ${SOURCES_MOC}
        ${SOURCES_UIS}
        ${SOURCES_RCS}
    )

    # OpenCOR plugins

    FOREACH(PLUGIN ${PLUGINS})
        TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            ${PLUGIN}Plugin
        )
    ENDFOREACH()

    # OpenCOR binaries

    FOREACH(PLUGIN_BINARY ${PLUGIN_BINARIES})
        TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            ${PLUGIN_BINARY}
        )
    ENDFOREACH()

    # Qt modules

    FOREACH(QT_MODULE ${QT_MODULES})
        FIND_PACKAGE(Qt5${QT_MODULE} REQUIRED)

        TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            Qt5::${QT_MODULE}
        )
    ENDFOREACH()

    # Linker settings

    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${PLUGIN_NAME}
        LINK_FLAGS "${LINK_FLAGS_PROPERTIES}"
    )

    # External binaries

    IF(NOT "${EXTERNAL_BINARIES_DIR}" STREQUAL "")
        FOREACH(EXTERNAL_BINARY ${EXTERNAL_BINARIES})
            TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                ${EXTERNAL_BINARIES_DIR}/${EXTERNAL_BINARY}
            )
        ENDFOREACH()
    ENDIF()

    # Location of our plugin

    STRING(REPLACE "${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/" "" PLUGIN_BUILD_DIR ${PROJECT_SOURCE_DIR})
    SET(PLUGIN_BUILD_DIR ${CMAKE_BINARY_DIR}/${PLUGIN_BUILD_DIR})

    IF(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
        SET(PLUGIN_BUILD_DIR ${PLUGIN_BUILD_DIR}/${CMAKE_CFG_INTDIR})
    ENDIF()

    # Keep track of the location of our plugins, so that we can properly clean
    # up our bundle on OS X

    LIST(APPEND PLUGIN_BUILD_DIRS ${PLUGIN_BUILD_DIR})

    SET(PLUGIN_BUILD_DIRS "${PLUGIN_BUILD_DIRS}" PARENT_SCOPE)

    # Copy the plugin to our plugins directory
    # Note: this is done so that we can, on Windows and Linux, test the use of
    #       plugins in OpenCOR without first having to package OpenCOR...

    SET(PLUGIN_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BUILD_DIR}/${PLUGIN_FILENAME}
                                                        ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME})

    # Also copy the plugin to our main build directory, since this is where any
    # test that requires the plugin will expect it to be

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BUILD_DIR}/${PLUGIN_FILENAME}
                                                        ${PROJECT_BUILD_DIR}/${PLUGIN_FILENAME})

    # A few OS X specific things

    IF(APPLE)
        # Clean up our plugin

        OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES(${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/PlugIns/${CMAKE_PROJECT_NAME}
                                             ${PLUGIN_FILENAME} ${QT_LIBRARIES})

        # Make sure that the plugin refers to our embedded version of the other
        # plugins on which it depends

        FOREACH(PLUGIN ${PLUGINS})
            # We don't know where the plugin is located, so we try our different
            # plugin build directories

            FOREACH(PLUGIN_BUILD_DIR ${PLUGIN_BUILD_DIRS})
                ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                                   COMMAND install_name_tool -change @rpath/${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN}${CMAKE_SHARED_LIBRARY_SUFFIX}
                                                                     @executable_path/../PlugIns/${CMAKE_PROJECT_NAME}/${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN}${CMAKE_SHARED_LIBRARY_SUFFIX}
                                                                     ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/PlugIns/${CMAKE_PROJECT_NAME}/${PLUGIN_FILENAME})
            ENDFOREACH()
        ENDFOREACH()

        # Make sure that the plugin refers to our embedded version of the
        # binary plugins on which it depends

        FOREACH(PLUGIN_BINARY ${PLUGIN_BINARIES})
            STRING(REGEX REPLACE "^.*/" "" PLUGIN_BINARY "${PLUGIN_BINARY}")

            ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                               COMMAND install_name_tool -change ${PLUGIN_BINARY}
                                                                 @executable_path/../PlugIns/${CMAKE_PROJECT_NAME}/${PLUGIN_BINARY}
                                                                 ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/PlugIns/${CMAKE_PROJECT_NAME}/${PLUGIN_FILENAME})
        ENDFOREACH()

        # Make sure that the plugin refers to our embedded version of the
        # external binaries on which it depends

        FOREACH(EXTERNAL_BINARY ${EXTERNAL_BINARIES})
            ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                               COMMAND install_name_tool -change ${EXTERNAL_BINARY}
                                                                 @executable_path/../Frameworks/${EXTERNAL_BINARY}
                                                                 ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/PlugIns/${CMAKE_PROJECT_NAME}/${PLUGIN_FILENAME})
        ENDFOREACH()
    ENDIF()

    # Package the plugin, but only if we are not on OS X since it will  have
    # already been copied

    IF(NOT APPLE)
        INSTALL(FILES ${PLUGIN_BUILD_DIR}/${PLUGIN_FILENAME}
                DESTINATION plugins/${CMAKE_PROJECT_NAME})
    ENDIF()

    # Create some tests, if any and if required

    IF(ENABLE_TESTS)
        FOREACH(TEST ${TESTS})
            SET(TEST_NAME ${PLUGIN_NAME}_${TEST})

            SET(TEST_SOURCE tests/${TEST}.cpp)
            SET(TEST_HEADER_MOC tests/${TEST}.h)

            IF(    EXISTS ${PROJECT_SOURCE_DIR}/${TEST_SOURCE}
               AND EXISTS ${PROJECT_SOURCE_DIR}/${TEST_HEADER_MOC})
                # The test exists, so build it

                QT5_WRAP_CPP(TEST_SOURCES_MOC
                    ../../plugin.h
                    ../../pluginmanager.h

                    ${TEST_HEADER_MOC}
                )

                ADD_EXECUTABLE(${TEST_NAME}
                    ../../../../tests/testsutils.cpp

                    ../../plugin.cpp
                    ../../plugininfo.cpp
                    ../../pluginmanager.cpp

                    ${SOURCES}
                    ${SOURCES_MOC}
                    ${SOURCES_UIS}
                    ${SOURCES_RCS}

                    ${TEST_SOURCE}
                    ${TEST_SOURCES_MOC}
                )

                # Plugins

                FOREACH(PLUGIN ${PLUGINS} ${PLUGIN_NAME})
                    TARGET_LINK_LIBRARIES(${TEST_NAME}
                        ${PLUGIN}Plugin
                    )
                ENDFOREACH()

                # OpenCOR binaries

                FOREACH(PLUGIN_BINARY ${PLUGIN_BINARIES})
                    TARGET_LINK_LIBRARIES(${TEST_NAME}
                        ${PLUGIN_BINARY}
                    )
                ENDFOREACH()

                # Qt modules

                FOREACH(QT_MODULE ${QT_MODULES} Test)
                    TARGET_LINK_LIBRARIES(${TEST_NAME}
                        Qt5::${QT_MODULE}
                    )
                ENDFOREACH()

                # Linker settings

                SET_TARGET_PROPERTIES(${TEST_NAME} PROPERTIES
                    OUTPUT_NAME ${TEST_NAME}
                    LINK_FLAGS "${LINK_FLAGS_PROPERTIES}"
                )

                # External binaries

                IF(NOT "${EXTERNAL_BINARIES_DIR}" STREQUAL "")
                    FOREACH(EXTERNAL_BINARY ${EXTERNAL_BINARIES})
                        TARGET_LINK_LIBRARIES(${TEST_NAME}
                            ${EXTERNAL_BINARIES_DIR}/${EXTERNAL_BINARY}
                        )
                    ENDFOREACH()
                ENDIF()

                # Copy the test to our tests directory
                # Note: DEST_TESTS_DIR is defined in our main CMake file...

                SET(TEST_FILENAME ${TEST_NAME}${CMAKE_EXECUTABLE_SUFFIX})

                ADD_CUSTOM_COMMAND(TARGET ${TEST_NAME} POST_BUILD
                                   COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/${TEST_FILENAME}
                                                                    ${DEST_TESTS_DIR}/${TEST_FILENAME})
            ELSE()
                MESSAGE(AUTHOR_WARNING "The '${TEST}' test for the '${PLUGIN_NAME}' plugin doesn't exist...")
            ENDIF()
        ENDFOREACH()
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(ADD_PLUGIN_BINARY PLUGIN_NAME)
    # Various initialisations

    SET(PLUGIN_NAME ${PLUGIN_NAME})

    SET(INCLUDE_DIRS)
    SET(QT_LIBRARIES)

    # Analyse the extra parameters

    SET(TYPE_OF_PARAMETER 0)

    FOREACH(PARAMETER ${ARGN})
        IF("${PARAMETER}" STREQUAL "INCLUDE_DIRS")
            SET(TYPE_OF_PARAMETER 1)
        ELSEIF("${PARAMETER}" STREQUAL "QT_LIBRARIES")
            SET(TYPE_OF_PARAMETER 2)
        ELSE()
            # Not one of the headers, so add the parameter to the corresponding
            # set

            IF(${TYPE_OF_PARAMETER} EQUAL 1)
                LIST(APPEND INCLUDE_DIRS ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 2)
                LIST(APPEND QT_LIBRARIES ${PARAMETER})
            ENDIF()
        ENDIF()
    ENDFOREACH()

    # Various include directories

    SET(PLUGIN_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)

    INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

    # Location of our plugins

    SET(PLUGIN_BINARY_DIR ${PROJECT_SOURCE_DIR}/bin/${DISTRIB_BINARY_DIR})

    # Copy the plugin to our plugins directory
    # Note #1: this is done so that we can, on Windows and Linux, test the use
    #          of plugins in OpenCOR without first having to package and deploy
    #          everything...
    # Note #2: to use ADD_CUSTOM_COMMAND() on Windows or Linux with Ninja
    #          doesn't work, so we use EXECUTE_PROCESS() instead...
    # Note #3: we don't use EXECUTE_PROCESS() on OS X because it doesn't work
    #          with Xcode...

    SET(PLUGIN_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

    IF(APPLE)
        ADD_CUSTOM_COMMAND(OUTPUT ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME}
                           COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BINARY_DIR}/${PLUGIN_FILENAME}
                                                            ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME})
    ELSE()
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BINARY_DIR}/${PLUGIN_FILENAME}
                                                         ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME})
    ENDIF()

    # Make a copy of the plugin to our main build directory

    IF(APPLE)
        ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BUILD_DIR}/${PLUGIN_FILENAME}
                           COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BINARY_DIR}/${PLUGIN_FILENAME}
                                                            ${PROJECT_BUILD_DIR}/${PLUGIN_FILENAME})
    ELSE()
        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BINARY_DIR}/${PLUGIN_FILENAME}
                                                         ${PROJECT_BUILD_DIR}/${PLUGIN_FILENAME})
    ENDIF()

    # A few OS X specific things

    IF(APPLE)
        # Make sure that the copy of our plugin in our plugins directory refers
        # to the system version of the Qt libraries on which it depends, this in
        # case we are not to package OpenCOR
        # Note: see OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES() for the reason...

        IF(NOT "$ENV{PACKAGE_OPENCOR}" STREQUAL "True")
            FOREACH(QT_LIBRARY ${QT_LIBRARIES})
                ADD_CUSTOM_TARGET(${PLUGIN_NAME}_${QT_LIBRARY}_UPDATE_OS_X_QT_REFERENCE_IN_BUNDLE ALL
                                  DEPENDS ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME}
                                  COMMAND install_name_tool -change @executable_path/../Frameworks/${QT_LIBRARY}.framework/Versions/${QT_VERSION_MAJOR}/${QT_LIBRARY}
                                                                    ${QT_LIBRARY_DIR}/${QT_LIBRARY}.framework/Versions/${QT_VERSION_MAJOR}/${QT_LIBRARY}
                                                                    ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME})
            ENDFOREACH()
        ENDIF()

        # Make sure that the copy of our plugin in our main build directory
        # refers to the system version of the Qt libraries on which it depends
        # Note: indeed, right now, it refers to our embedded version of the Qt
        #       libraries while, if we want the tests to work, it should refer
        #       to the system version of the Qt libraries, so...

        FOREACH(QT_LIBRARY ${QT_LIBRARIES})
            ADD_CUSTOM_TARGET(${PLUGIN_NAME}_${QT_LIBRARY}_UPDATE_OS_X_QT_REFERENCE_IN_BUILD_DIRECTORY ALL
                              DEPENDS ${PROJECT_BUILD_DIR}/${PLUGIN_FILENAME}
                              COMMAND install_name_tool -change @executable_path/../Frameworks/${QT_LIBRARY}.framework/Versions/${QT_VERSION_MAJOR}/${QT_LIBRARY}
                                                                ${QT_LIBRARY_DIR}/${QT_LIBRARY}.framework/Versions/${QT_VERSION_MAJOR}/${QT_LIBRARY}
                                                                ${PROJECT_BUILD_DIR}/${PLUGIN_FILENAME})
        ENDFOREACH()
    ENDIF()

    # Package the plugin, but only if we are not on OS X since it will have
    # already been copied

    IF(NOT APPLE)
        INSTALL(FILES ${PLUGIN_BINARY_DIR}/${PLUGIN_FILENAME}
                DESTINATION plugins/${CMAKE_PROJECT_NAME})
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(COPY_FILE_TO_BUILD_DIR PROJECT_TARGET ORIG_DIRNAME DEST_DIRNAME FILENAME)
    # Determine the real destination folder

    IF("${ARGN}" STREQUAL "")
        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy ${ORIG_DIRNAME}/${FILENAME}
                                                            ${PROJECT_BUILD_DIR}/${DEST_DIRNAME}/${FILENAME})
    ELSE()
        # An argument was passed so use it to rename the file which is to be
        # copied

        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy ${ORIG_DIRNAME}/${FILENAME}
                                                            ${PROJECT_BUILD_DIR}/${DEST_DIRNAME}/${ARGN})
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(WINDOWS_DEPLOY_QT_LIBRARIES)
    FOREACH(LIBRARY ${ARGN})
        # Deploy the Qt library

        INSTALL(FILES ${QT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${LIBRARY}${CMAKE_SHARED_LIBRARY_SUFFIX}
                DESTINATION bin)
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(WINDOWS_DEPLOY_QT_PLUGIN PLUGIN_DIRECTORY PLUGIN_CATEGORY)
    FOREACH(PLUGIN_NAME ${ARGN})
        # Deploy the Qt plugin

        INSTALL(FILES ${QT_PLUGINS_DIR}/${PLUGIN_CATEGORY}/${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
                DESTINATION ${PLUGIN_DIRECTORY}/${PLUGIN_CATEGORY})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(WINDOWS_DEPLOY_LIBRARY DIRNAME FILENAME)
    # Copy the library file to both the build and build/bin folders, so we can
    # test things without first having to deploy OpenCOR

    COPY_FILE_TO_BUILD_DIR(${PROJECT_NAME} ${DIRNAME} . ${FILENAME})
    COPY_FILE_TO_BUILD_DIR(${PROJECT_NAME} ${DIRNAME} bin ${FILENAME})

    # Install the library file

    INSTALL(FILES ${DIRNAME}/${FILENAME}
            DESTINATION bin)
ENDMACRO()

#===============================================================================

MACRO(LINUX_DEPLOY_QT_PLUGIN PLUGIN_CATEGORY)
    FOREACH(PLUGIN_NAME ${ARGN})
        # Deploy the Qt plugin

        INSTALL(FILES ${QT_PLUGINS_DIR}/${PLUGIN_CATEGORY}/${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
                DESTINATION plugins/${PLUGIN_CATEGORY})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(LINUX_DEPLOY_LIBRARY DIRNAME FILENAME)
    # Copy the library file to the build folder, so we can test things without
    # first having to deploy OpenCOR

    COPY_FILE_TO_BUILD_DIR(${PROJECT_NAME} ${DIRNAME} . ${FILENAME})

    # Install the library file

    INSTALL(FILES ${DIRNAME}/${FILENAME}
            DESTINATION lib)
ENDMACRO()

#===============================================================================

MACRO(OS_X_QT_LIBRARIES FILENAME QT_LIBRARIES)
    # Retrieve the file's full-path Qt libraries as a list

    SET(QT_LIBRARY_DIR_FOR_GREP "\t${QT_LIBRARY_DIR}/")

    EXECUTE_PROCESS(COMMAND otool -L ${FILENAME}
                    COMMAND grep --colour=never ${QT_LIBRARY_DIR_FOR_GREP}
                    OUTPUT_VARIABLE RAW_QT_LIBRARIES)

    STRING(REPLACE "\n" ";" RAW_QT_LIBRARIES "${RAW_QT_LIBRARIES}")

    # Extract and return the Qt depencies as a list

    SET(${QT_LIBRARIES})

    FOREACH(RAW_QT_LIBRARY ${RAW_QT_LIBRARIES})
        STRING(REPLACE ${QT_LIBRARY_DIR_FOR_GREP} "" RAW_QT_LIBRARY "${RAW_QT_LIBRARY}")
        STRING(REGEX REPLACE "\\.framework.*$" "" QT_LIBRARY "${RAW_QT_LIBRARY}")

        LIST(APPEND ${QT_LIBRARIES} ${QT_LIBRARY})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES DIRNAME FILENAME)
    # Strip the Qt file of all local symbols

    SET(FULL_FILENAME ${DIRNAME}/${FILENAME})

    IF(RELEASE_MODE)
        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                           COMMAND strip -x ${FULL_FILENAME})
    ENDIF()

    # Clean up the Qt file's id

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND install_name_tool -id ${FILENAME}
                                                     ${FULL_FILENAME})

    # Make sure that the Qt file refers to our embedded version of its Qt
    # dependencies
    # Note: we only do this if we are to package OpenCOR. Indeed, if we were to
    #       do this when building OpenCOR for, say, debugging purposes, then we
    #       would end up with two sets of Qt libraries (one in /Applications/Qt5
    #       and another in the OpenCOR bundle), potentially confusing tools such
    #       as Xcode, generating messages like "XXX is implemented in both YYY
    #       and ZZZ. One of the two will be used. Which one is undefined." even
    #       though everything is actually fine...

    IF("$ENV{PACKAGE_OPENCOR}" STREQUAL "True")
        FOREACH(DEPENDENCY ${ARGN})
            SET(DEPENDENCY_FILENAME ${DEPENDENCY}.framework/Versions/${QT_VERSION_MAJOR}/${DEPENDENCY})

            ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                               COMMAND install_name_tool -change ${QT_LIBRARY_DIR}/${DEPENDENCY_FILENAME}
                                                                 @executable_path/../Frameworks/${DEPENDENCY_FILENAME}
                                                                 ${FULL_FILENAME})
        ENDFOREACH()
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(OS_X_DEPLOY_QT_FILE ORIG_DIRNAME DEST_DIRNAME FILENAME)
    # Copy the Qt file

    SET(ORIG_FILENAME ${ORIG_DIRNAME}/${FILENAME})

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${ORIG_FILENAME}
                                                        ${DEST_DIRNAME}/${FILENAME})

    # Retrieve the Qt file's Qt dependencies

    OS_X_QT_LIBRARIES(${ORIG_FILENAME} DEPENDENCIES)

    # Clean up the Qt file

    OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES(${DEST_DIRNAME} ${FILENAME} ${DEPENDENCIES})
ENDMACRO()

#===============================================================================

MACRO(OS_X_DEPLOY_QT_LIBRARIES)
    FOREACH(LIBRARY_NAME ${ARGN})
        # Deploy the Qt library

        SET(QT_FRAMEWORK_DIR ${LIBRARY_NAME}.framework/Versions/${QT_VERSION_MAJOR})

        OS_X_DEPLOY_QT_FILE(${QT_LIBRARY_DIR}/${QT_FRAMEWORK_DIR}
                            ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Frameworks/${QT_FRAMEWORK_DIR}
                            ${LIBRARY_NAME})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(OS_X_DEPLOY_QT_PLUGIN PLUGIN_CATEGORY)
    FOREACH(PLUGIN_NAME ${ARGN})
        # Deploy the Qt plugin

        OS_X_DEPLOY_QT_FILE(${QT_PLUGINS_DIR}/${PLUGIN_CATEGORY}
                            ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/PlugIns/${PLUGIN_CATEGORY}
                            ${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(OS_X_DEPLOY_LIBRARY DIRNAME LIBRARY_NAME)
    # Copy the library

    SET(LIBRARY_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${LIBRARY_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
    SET(LIBRARY_FILEPATH ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Frameworks/${LIBRARY_FILENAME})

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${DIRNAME}/${LIBRARY_FILENAME}
                                                        ${LIBRARY_FILEPATH})

    # Copy the library to the build directory, so that we can test any plugin
    # that has a dependency on it

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${DIRNAME}/${LIBRARY_FILENAME}
                                                        ${PROJECT_BUILD_DIR}/${LIBRARY_FILENAME})

    # Strip the library of all local symbols

    IF(RELEASE_MODE)
        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                           COMMAND strip -x ${LIBRARY_FILEPATH})
    ENDIF()

    # Make sure that the library refers to our embedded version of the libraries
    # on which it depends

    FOREACH(DEPENDENCY_NAME ${ARGN})
        SET(DEPENDENCY_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${DEPENDENCY_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                           COMMAND install_name_tool -change ${DEPENDENCY_FILENAME}
                                                             @executable_path/../Frameworks/${DEPENDENCY_FILENAME}
                                                             ${LIBRARY_FILEPATH})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(RETRIEVE_BINARY_FILE DIRNAME FILENAME SHA1_VALUE)
    # Create the destination folder, if needed

    SET(REAL_DIRNAME ${CMAKE_SOURCE_DIR}/${DIRNAME})

    IF(NOT EXISTS ${REAL_DIRNAME})
        FILE(MAKE_DIRECTORY ${REAL_DIRNAME})
    ENDIF()

    # Make sure that the file, if it exists, has the expected SHA-1 value

    SET(REAL_FILENAME ${REAL_DIRNAME}/${FILENAME})

    IF(EXISTS ${REAL_FILENAME})
        FILE(SHA1 ${REAL_FILENAME} REAL_SHA1_VALUE)

        IF(NOT "${REAL_SHA1_VALUE}" STREQUAL "${SHA1_VALUE}")
            # The file doesn't have the expected SHA-1 value, so remove it

            FILE(REMOVE ${REAL_FILENAME})
        ENDIF()
    ENDIF()

    # Retrieve the file from the OpenCOR website, if needed
    # Note: we would normally provide the SHA-1 value to the FILE(DOWNLOAD)
    #       call, but this would create an empty file to start with and if the
    #       file cannot be downloaded for some reason or another, then we would
    #       still have that file and CMake would then complain about its SHA-1
    #       value being wrong (as well as not being able to download the file),
    #       so we handle everything ourselves...

    IF(NOT EXISTS ${REAL_FILENAME})
        MESSAGE("Retrieving '${DIRNAME}/${FILENAME}'...")

        # We retrieve the compressed version of the file

        SET(COMPRESSED_FILENAME ${FILENAME}.tar.gz)
        SET(REAL_COMPRESSED_FILENAME ${REAL_DIRNAME}/${COMPRESSED_FILENAME})

        FILE(DOWNLOAD "http://www.opencor.ws/binaries/${DIRNAME}/${COMPRESSED_FILENAME}" ${REAL_COMPRESSED_FILENAME}
             SHOW_PROGRESS STATUS STATUS)

        # Uncompress the compressed version of the file, should we have managed
        # to retrieve it

        LIST(GET STATUS 0 STATUS_CODE)

        IF(${STATUS_CODE} EQUAL 0)
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar zxvf ${REAL_COMPRESSED_FILENAME}
                            WORKING_DIRECTORY ${REAL_DIRNAME} OUTPUT_QUIET)
            FILE(REMOVE ${REAL_COMPRESSED_FILENAME})
        ELSE()
            FILE(REMOVE ${REAL_COMPRESSED_FILENAME})
            # Note: this is in case we had an HTTP error of sorts, in which case
            #       we would end up with an empty file...

            MESSAGE(FATAL_ERROR "The compressed version of the file could not be retrieved...")
        ENDIF()

        # Check that the file, if we managed to retrieve it, has the expected
        # SHA-1 value

        IF(EXISTS ${REAL_FILENAME})
            FILE(SHA1 ${REAL_FILENAME} REAL_SHA1_VALUE)

            IF(NOT "${REAL_SHA1_VALUE}" STREQUAL "${SHA1_VALUE}")
                FILE(REMOVE ${REAL_FILENAME})

                MESSAGE(FATAL_ERROR "The file does not have the expected SHA-1 value...")
            ENDIF()
        ELSE()
            FILE(REMOVE ${REAL_COMPRESSED_FILENAME})

            MESSAGE(FATAL_ERROR "The file could not be uncompressed...")
        ENDIF()
    ENDIF()
ENDMACRO()
