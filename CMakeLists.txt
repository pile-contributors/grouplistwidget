if (NOT GROUPLISTWIDGET_BUILD_MODE)
    set (GROUPLISTWIDGET_BUILD_MODE STATIC)
endif ()

include(pile_support)
pileInclude (GroupListWidget)
grouplistwidgetInit(${GROUPLISTWIDGET_BUILD_MODE})
