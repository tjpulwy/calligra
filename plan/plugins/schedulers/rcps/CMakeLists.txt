add_definitions(-DTRANSLATION_DOMAIN=\"calligraplan_scheduler_rcps\")

set(LIBRCPS_INCLUDE_DIR 3rdparty/LibRCPS/src)
include_directories( ${LIBRCPS_INCLUDE_DIR} ${KOODF_INCLUDES} ${CMAKE_SOURCE_DIR}/plan/libs/kernel  )

add_subdirectory( 3rdparty )
add_subdirectory( tests )

set ( RCPSScheduler_SRCS
    KPlatoRCPSPlugin.cpp
    KPlatoRCPSScheduler.cpp
)

add_library(kplatorcpsscheduler MODULE ${RCPSScheduler_SRCS} )
calligraplan_scheduler_desktop_to_json(kplatorcpsscheduler planrcpsscheduler.desktop)

target_link_libraries(
    kplatorcpsscheduler
    kplatokernel
    rcps_plan
#    ${LIBRCPS_LIBRARIES}
)

install( TARGETS kplatorcpsscheduler DESTINATION ${PLUGIN_INSTALL_DIR}/calligraplan/schedulers )
