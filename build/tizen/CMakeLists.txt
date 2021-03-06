CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(dali-demo C CXX)

SET(dali-demo_VERSION_MAJOR 1)
SET(dali-demo_VERSION_MINOR 0)

SET(ROOT_SRC_DIR ${CMAKE_SOURCE_DIR}/../..)
SET(DEMO_SHARED ${CMAKE_SOURCE_DIR}/../../shared)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})

IF(DEFINED DALI_APP_DIR)
        SET(BINDIR ${DALI_APP_DIR}/bin)
ELSE()
        SET(BINDIR ${PREFIX}/bin)
ENDIF()

IF(DEFINED DALI_APP_RES_DIR)
        SET(APP_DATA_RES_DIR ${DALI_APP_RES_DIR})
ELSE()
        SET(APP_DATA_RES_DIR ${PREFIX}/share/com.samsung.dali-demo/res)
ENDIF()

MESSAGE( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )
if(CMAKE_BUILD_TYPE MATCHES Debug)
    MESSAGE("Setting variable: DEBUG_ENABLED")
    SET(DEBUG_ENABLED "1")
ENDIF()

SET(RESOURCE_DIR ${ROOT_SRC_DIR}/resources)
SET(LOCAL_IMAGES_DIR ${RESOURCE_DIR}/images)
SET(LOCAL_VIDEOS_DIR ${RESOURCE_DIR}/videos)
SET(LOCAL_MODELS_DIR ${RESOURCE_DIR}/models)
SET(LOCAL_SCRIPTS_DIR ${RESOURCE_DIR}/scripts)
IF(NOT DEFINED LOCAL_STYLE_DIR)
        SET(LOCAL_STYLE_DIR ${RESOURCE_DIR}/style)
ENDIF()

SET(IMAGES_DIR ${APP_DATA_RES_DIR}/images/)
SET(VIDEOS_DIR ${APP_DATA_RES_DIR}/videos/)
SET(MODELS_DIR ${APP_DATA_RES_DIR}/models/)
SET(SCRIPTS_DIR ${APP_DATA_RES_DIR}/scripts/)
SET(STYLE_DIR ${APP_DATA_RES_DIR}/style/)

IF(NOT DEFINED LOCALE_DIR)
        SET(LOCALE_DIR ${PREFIX}/share/locale)
ENDIF()

SET(DEMO_IMAGE_DIR \\"${IMAGES_DIR}\\")
SET(DEMO_VIDEO_DIR \\"${VIDEOS_DIR}\\")
SET(DEMO_MODEL_DIR \\"${MODELS_DIR}\\")
SET(DEMO_SCRIPT_DIR \\"${SCRIPTS_DIR}\\")
SET(DEMO_STYLE_DIR \\"${STYLE_DIR}\\")
SET(DEMO_THEME_PATH \\"${STYLE_DIR}demo-theme.json\\")
SET(DEMO_EXAMPLE_BIN \\"${BINDIR}/\\")
SET(DEMO_LOCALE_DIR \\"${LOCALE_DIR}\\")
SET(DEMO_LANG \\"${LANG}\\")

SET(DEMO_STYLE_IMAGE_DIR ${STYLE_DIR}/images)

FILE(GLOB LOCAL_IMAGES_PNG RELATIVE  "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.png")
FILE(GLOB LOCAL_IMAGES_JPG RELATIVE  "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.jpg")
FILE(GLOB LOCAL_IMAGES_GIF RELATIVE  "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.gif")
FILE(GLOB LOCAL_IMAGES_BMP RELATIVE  "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.bmp")
FILE(GLOB LOCAL_IMAGES_ICO RELATIVE  "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.ico")
FILE(GLOB LOCAL_IMAGES_WBMP RELATIVE "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.wbmp")
FILE(GLOB LOCAL_IMAGES_KTX RELATIVE  "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.ktx")
FILE(GLOB LOCAL_IMAGES_ASTC RELATIVE "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.astc")
FILE(GLOB LOCAL_IMAGES_SVG RELATIVE  "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.svg")

SET(LOCAL_IMAGES_LIST ${LOCAL_IMAGES_PNG};${LOCAL_IMAGES_JPG};${LOCAL_IMAGES_GIF};${LOCAL_IMAGES_BMP};${LOCAL_IMAGES_ICO};${LOCAL_IMAGES_WBMP};${LOCAL_IMAGES_KTX};${LOCAL_IMAGES_ASTC};${LOCAL_IMAGES_SVG})
FOREACH(flag ${LOCAL_IMAGES_LIST})
        INSTALL(FILES ${LOCAL_IMAGES_DIR}/${flag} DESTINATION ${IMAGES_DIR})
ENDFOREACH(flag)

FILE(GLOB LOCAL_VIDEOS_LIST RELATIVE "${LOCAL_VIDEOS_DIR}" "${LOCAL_VIDEOS_DIR}/*")
FOREACH(flag ${LOCAL_VIDEOS_LIST})
        INSTALL(FILES ${LOCAL_VIDEOS_DIR}/${flag} DESTINATION ${VIDEOS_DIR})
ENDFOREACH(flag)

FILE(GLOB LOCAL_MODELS_LIST RELATIVE "${LOCAL_MODELS_DIR}" "${LOCAL_MODELS_DIR}/*")
FOREACH(flag ${LOCAL_MODELS_LIST})
        INSTALL(FILES ${LOCAL_MODELS_DIR}/${flag} DESTINATION ${MODELS_DIR})
ENDFOREACH(flag)

FILE(GLOB LOCAL_SCRIPTS_LIST RELATIVE "${LOCAL_SCRIPTS_DIR}" "${LOCAL_SCRIPTS_DIR}/*.json")
FOREACH(flag ${LOCAL_SCRIPTS_LIST})
        INSTALL(FILES ${LOCAL_SCRIPTS_DIR}/${flag} DESTINATION ${SCRIPTS_DIR})
ENDFOREACH(flag)

#Create resources location file
CONFIGURE_FILE( resources-location.in ${DEMO_SHARED}/resources-location.cpp )

#Replace @DEMO_STYLE_IMAGE_DIR@ in following files
CONFIGURE_FILE( ${LOCAL_STYLE_DIR}/demo-theme.json.in ${LOCAL_STYLE_DIR}/demo-theme.json )
CONFIGURE_FILE( ${LOCAL_STYLE_DIR}/style-example-theme-one.json.in ${LOCAL_STYLE_DIR}/style-example-theme-one.json )
CONFIGURE_FILE( ${LOCAL_STYLE_DIR}/style-example-theme-two.json.in ${LOCAL_STYLE_DIR}/style-example-theme-two.json )
CONFIGURE_FILE( ${LOCAL_STYLE_DIR}/style-example-theme-three.json.in ${LOCAL_STYLE_DIR}/style-example-theme-three.json )
MESSAGE("Configured ${LOCAL_STYLE_DIR}/style-example-theme<>.json files")

FILE(GLOB LOCAL_STYLES_LIST "${LOCAL_STYLE_DIR}/*.json")
FOREACH(flag ${LOCAL_STYLES_LIST})
  INSTALL(FILES ${flag} DESTINATION ${STYLE_DIR})
ENDFOREACH(flag)
MESSAGE("Installed ${LOCAL_STYLES_LIST} to ${STYLE_DIR}")

FILE(GLOB LOCAL_STYLE_IMAGES_LIST "${LOCAL_STYLE_DIR}/images/*.png")
FOREACH(flag ${LOCAL_STYLE_IMAGES_LIST})
  INSTALL(FILES ${flag} DESTINATION ${STYLE_DIR}/images)
ENDFOREACH(flag)
MESSAGE("Installed style resources ${LOCAL_STYLE_IMAGES_LIST} to ${STYLE_DIR}/images")

SET(PKG_LIST dali-core
             dali-adaptor
             dali-toolkit )

INCLUDE(FindPkgConfig)
pkg_check_modules(REQUIRED_PKGS REQUIRED ${PKG_LIST})

FOREACH(flag ${REQUIRED_PKGS_CFLAGS})
        SET(REQUIRED_CFLAGS "${REQUIRED_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(DALI_DEMO_CFLAGS "-DDEMO_IMAGE_DIR=${DEMO_IMAGE_DIR} -DDEMO_VIDEO_DIR=${DEMO_VIDEO_DIR} -DDEMO_MODEL_DIR=${DEMO_MODEL_DIR} -DDEMO_SCRIPT_DIR=${DEMO_SCRIPT_DIR} -DDEMO_STYLE_DIR=${DEMO_STYLE_DIR} -DDEMO_THEME_PATH=${DEMO_THEME_PATH} -DDEMO_EXAMPLE_BIN=${DEMO_EXAMPLE_BIN} -DDEMO_LOCALE_DIR=${DEMO_LOCALE_DIR} -fvisibility=hidden -DHIDE_DALI_INTERNALS -DDEMO_LANG=${DEMO_LANG}")

###########################################################################
# Internationalization

SET(PO_DIR ${RESOURCE_DIR}/po)

FILE(GLOB PO_FILES RELATIVE "${PO_DIR}" "${PO_DIR}/*.po")

SET(MSGFMT "/usr/bin/msgfmt")
SET(MO_FILES_DIR ${CMAKE_BINARY_DIR}/mo)
FILE(MAKE_DIRECTORY ${MO_FILES_DIR})

FOREACH(PO_FILE ${PO_FILES})
        SET(PO_FILE ${PO_DIR}/${PO_FILE})
        MESSAGE("PO: ${PO_FILE}")
        GET_FILENAME_COMPONENT(ABS_PO_FILE ${PO_FILE} ABSOLUTE)
        GET_FILENAME_COMPONENT(lang ${ABS_PO_FILE} NAME_WE)
        SET(MO_FILE ${MO_FILES_DIR}/${lang}.mo)
        ADD_CUSTOM_COMMAND(OUTPUT ${MO_FILE}
                           COMMAND ${MSGFMT} -o ${MO_FILE} ${ABS_PO_FILE}
                           DEPENDS ${ABS_PO_FILE})
        INSTALL(FILES ${MO_FILE} DESTINATION ${LOCALE_DIR}/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
        SET(MO_FILES ${MO_FILES} ${MO_FILE})
ENDFOREACH(PO_FILE)

MESSAGE(".mo files: ${MO_FILES}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${MO_FILES})

OPTION(INTERNATIONALIZATION "Internationalization demo string names" ON)
IF (INTERNATIONALIZATION)
        SET(DALI_DEMO_CFLAGS "${DALI_DEMO_CFLAGS} -DINTERNATIONALIZATION_ENABLED")
ENDIF(INTERNATIONALIZATION)

###########################################################################

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${REQUIRED_CFLAGS} ${DALI_DEMO_CFLAGS} -Werror -Wall -fPIE")
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

INCLUDE_DIRECTORIES(${ROOT_SRC_DIR})
#INCLUDE_DIRECTORIES(${DEMO_SRC_DIR})

ADD_SUBDIRECTORY(demo)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(builder)
