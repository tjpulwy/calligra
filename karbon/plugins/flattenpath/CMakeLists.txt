include_directories( ${CMAKE_SOURCE_DIR}/karbon )


########### next target ###############

set(karbon_flattenpath_PART_SRCS FlattenPathPlugin.cpp KarbonPathFlattenCommand.cpp )


add_library(karbon_flattenpath MODULE ${karbon_flattenpath_PART_SRCS})

karbon_viewplugin_desktop_to_json(karbon_flattenpath karbon_flattenpath.desktop)

target_link_libraries(karbon_flattenpath karboncommon)

install(TARGETS karbon_flattenpath  DESTINATION ${PLUGIN_INSTALL_DIR}/karbon/extensions)


########### install files ###############

install( FILES  FlattenPathPlugin.rc  DESTINATION ${DATA_INSTALL_DIR}/karbon/plugins)
