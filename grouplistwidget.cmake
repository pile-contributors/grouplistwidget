
# enable/disable cmake debug messages related to this pile
set (GROUPLISTWIDGET_DEBUG_MSG OFF)

# make sure support code is present; no harm
# in including it twice; the user, however, should have used
# pileInclude() from pile_support.cmake module.
include(pile_support)

# initialize this module
macro    (grouplistwidgetInit
          ref_cnt_use_mode)

    # default name
    if (NOT GROUPLISTWIDGET_INIT_NAME)
        set(GROUPLISTWIDGET_INIT_NAME "GroupListWidget")
    endif ()

    # compose the list of headers and sources
    set(GROUPLISTWIDGET_HEADERS
        "groupmodel.h"
        "grouplistdelegate.h"
        "groupcustomizer.h"
        "grouplistwidget.h")
    set(GROUPLISTWIDGET_SOURCES
        "groupmodel.cc"
        "grouplistdelegate.cc"
        "groupcustomizer.cc"
        "grouplistwidget.cc")
    set(GROUPLISTWIDGET_QT_MODS
        Core Widgets)

    pileSetSources(
        "${GROUPLISTWIDGET_INIT_NAME}"
        "${GROUPLISTWIDGET_HEADERS}"
        "${GROUPLISTWIDGET_SOURCES}")

    pileSetCommon(
        "${GROUPLISTWIDGET_INIT_NAME}"
        "0;0;1;d"
        "ON"
        "${ref_cnt_use_mode}"
        ""
        "category1"
        "tag1;tag2")

endmacro ()
