/**
 * @file grouplistwidget.cc
 * @brief Definitions for GROUPLISTWIDGET class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */
#include "grouplistdelegate.h"
#include "grouplistgroup.h"
#include "grouplistwidget.h"
#include "groupmodel.h"
#include "groupsubmodel.h"


/**
 * @class GroupListGroup
 *
 * .
 */


GroupListGroup::GroupListGroup (GroupListWidget * parent) :
    QListView (),
    parent_(parent)
{
    setUniformItemSizes (true);
#   if 1 // ndef GROUPLISTWIDGET_DEBUG
    setFrameStyle (QFrame::NoFrame | QFrame::Plain);
#   else
    setFrameStyle (QFrame::Box | QFrame::Plain);
#   endif
    setViewMode (parent_->viewMode());
    setResizeMode (QListView::Adjust);
    setMovement (QListView::Static);
    setFlow (parent_->flow());
    setWrapping (true);
    int c = parent_->underModel ()->label ().column ();
    if (c != -1)
        setModelColumn (c);
    QAbstractItemDelegate * del = parent_->listDelegate ();
    if (del != NULL)
        setItemDelegate (del);
    if (parent_->pixmapSize () != -1) {
        setGridSize (parent->gridCell ());
    }
}

void GroupListGroup::anchorVtable () const {}
