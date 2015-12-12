/**
 * @file grouplistwidget.cc
 * @brief Definitions for GroupListWidget class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "grouplistwidget.h"
#include "grouplistwidget-private.h"
#include "groupmodel.h"
#include <QListView>

class GrpTreeItem : public QTreeWidgetItem {
public:
    int gtroup_index_;


    GrpTreeItem (const QString & s_name, int idx) :
        QTreeWidgetItem (QStringList () << s_name ),
        gtroup_index_ (idx)
    {
        QTreeWidgetItem * tvi = new QTreeWidgetItem (this);
        // tvi->set
    }
};


/**
 * @class GroupListWidget
 *
 * A list of items with grouping.
 */

/* ------------------------------------------------------------------------- */
/**
 * Detailed description for constructor.
 */
GroupListWidget::GroupListWidget (QWidget *parent) :
    QTreeWidget (parent)
{
    GROUPLISTWIDGET_TRACE_ENTRY;

    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Detailed description for destructor.
 */
GroupListWidget::~GroupListWidget()
{
    GROUPLISTWIDGET_TRACE_ENTRY;

    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::setGroupModel (GroupModel *value)
{
    clear ();
    int i_max = value->groupCount();

    for (int i = 0; i < i_max; ++i) {
        GrpTreeItem * tvi = new GrpTreeItem (value->groupLabel (i), i);
        addTopLevelItem (tvi);
        QTreeWidgetItem * subtvi = new QTreeWidgetItem (this);
        // tvi->set
        QListView * lv = new QListView ();
        lv->setViewMode (QListView::IconMode);
        lv->setResizeMode (QListView::Adjust);
        lv->setMovement (QListView::Static);
        lv->setModel (value->groupList (i));
        setItemWidget (subtvi, 0, lv);
    }
}
/* ========================================================================= */
