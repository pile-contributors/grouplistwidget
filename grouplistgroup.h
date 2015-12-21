/**
 * @file dbmodel.h
 * @brief Declarations for DbModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_GROUP_LIST_GROUP_H_INCLUDE
#define GUARD_GROUP_LIST_GROUP_H_INCLUDE

#include <grouplistwidget/grouplistwidget-config.h>
#include <QListView>

class GroupListWidget;

//! Represents a list widget inside the tree presenting the content of a group.
class GROUPLISTWIDGET_EXPORT GroupListGroup : public QListView {
public:
    GroupListWidget * parent_;

    GroupListGroup (
            GroupListWidget * parent);

    QStyleOptionViewItem
    viewOptions() {
        return QListView::viewOptions();
    }

}; // class GroupListDelegate

#endif // GUARD_GROUP_LIST_GROUP_H_INCLUDE
