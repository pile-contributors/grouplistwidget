
/**
 * @file groupsubmodel.cc
 * @brief Definitions for GroupSubModel class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "groupsubmodel.h"
#include "groupmodel.h"
#include "grouplistwidget-private.h"
#include <QAbstractItemModel>
#include <QList>
#include <QVariant>
#include <QSize>

/**
 * @class GroupSubModel
 *
 * This is a simple proxy that can be installed in a common QListView.
 * It holds a mapping between the rows in this model and the rows
 * in GroupModel's base model. The rows are sorted according cu
 * the rule set for sorting in GroupModel.
 *
 * Inside the GroupModel each group is represented by one instance
 * of this class as it provides both the user-visible
 * label() and the groupKey() for grouping.
 */

/* ------------------------------------------------------------------------- */
/**
 *
 */
GroupSubModel::GroupSubModel (
        GroupModel *model, const QVariant & key, const QString & lbl) :
    QAbstractListModel (),
    m_(model),
    map_(),
    key_(key),
    s_label_(lbl),
    list_index_(-1)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 *
 */
GroupSubModel::~GroupSubModel()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Makes sure that the inserted row is placed at appropriate index.
 */
void GroupSubModel::insertSortedRecord (int row)
{
    int idx = 0;
    bool inserted = false;
    int sortc = m_->sortingColumn();
    if (sortc == -1) {
        // no sorting so we use original order
        // still, we have no guarantee that the rows arive in
        // increasing order
        foreach (int rold, map_) {
            if (rold > row) {
                map_.insert (idx, row);
                inserted = true;
            }
            ++idx;
        }
    } else {
        int sortr = m_->sortingRole ();
        GroupModel::Compare sort_func = m_->sortingFunc();
        QModelIndex midx_new = m_->baseModel()->index (row, sortc);
        QVariant new_data = midx_new.data (sortr);

        foreach (int row, map_) {
            QModelIndex midx_iter = m_->baseModel()->index (row, sortc);
            QVariant iter_data = midx_iter.data (sortr);

            GroupModel::ComparisionReslt res = sort_func (
                        m_, sortc, new_data, iter_data);

            switch (res) {
            case GroupModel::Equal: {
                map_.insert (idx+1, row);
                inserted = true;
                break; }
            case GroupModel::Smaller: {
                map_.insert (idx, row);
                inserted = true;
                break; }
            case GroupModel::Larger: {
                break; }
            }
            if (inserted)
                break;
            ++idx;
        }
    }
    if (!inserted) {
        map_.append (row);
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Retrieves the values for all rows and, based on those, arranges the internal
 * list of rows in ascending order.
 */
void GroupSubModel::performSorting ()
{
    int sortc = m_->sortingColumn();
    Q_ASSERT(sortc != -1);
    Q_ASSERT(m_->baseModel() != NULL);

    int i_max = map_.count();
    if (i_max == 0)
        return;
    int sortr = m_->sortingRole ();
    beginResetModel();

    // create a list of values so that we don't retrieve them multiple times
    QVariantList vlst;
    vlst.reserve (i_max);
    for (int i = 0; i < i_max; ++i) {
        QModelIndex midx = m_->baseModel()->index (map_.at (i), sortc);
        vlst.append (midx.data (sortr));
    }


    GroupModel::Compare sort_func = m_->sortingFunc();
    for (int n = 0; n < i_max; n++) {
        for (int i = n + 1; i < i_max; i++) {
            const QVariant & nval = vlst.at (n);
            const QVariant & ival = vlst.at (i);

            GroupModel::ComparisionReslt res = sort_func (
                        m_, sortc, nval, ival);
            switch (res) {
            case GroupModel::Equal: {
                // equal values ordered by row index
                if (map_.at (n) > map_.at (i)) {
                    vlst.swap (n, i);
                    map_.swap (n, i);
                }
                break; }
            case GroupModel::Smaller: {
                // ok
                break; }
            case GroupModel::Larger: {
                vlst.swap (n, i);
                map_.swap (n, i);
                break; }
            }
        }
    }
    endResetModel();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This would restore the original sorting from base model.
 */
void GroupSubModel::performUnsorting()
{
    beginResetModel();
    int i_max = map_.count();
    if (i_max == 0)
        return;

    for (int n = 0; n < i_max; n++) {
        for (int i = n + 1; i < i_max; i++) {
            if (map_.at (n) > map_.at (i)) {
                map_.swap (n, i);
            }
        }
    }
    endResetModel();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * As this is a list model it is expected that column will always be 0.
 * Nonetheless mosst of the time the column is forwarded to the user model.
 *
 * The method filters the decoration role for column 0 that is handled by
 * requesting the decoration from parent model.
 *
 */
QVariant GroupSubModel::data (const QModelIndex &index, int role) const
{
    int r = index.row();
    if ((r < 0) || (r >= map_.count())) {
        GROUPLISTWIDGET_DEBUGM(
                    "GroupSubModel data requested for non-existing row %d\n",
                    index.row());
        return QVariant ();
    }
    r = mapRowToBaseModel (r);

    int c = index.column();
    if (index.column() == 0) {
        if (role == Qt::DecorationRole) {
            // the icon
            c = m_->pixmapColumn();
            if (c == -1) {
                // all images are off
                return QVariant();
            }
            role = m_->pixmapRole();
        } /*else if (role == Qt::SizeHintRole) {
            return QSize(60, 20);
        }*/ else if (role >= GroupModel::BaseColRole) {
            c = role - GroupModel::BaseColRole;
            if (c >= m_->labelCount()) {
                return QVariant();
            }

            ModelId mid = m_->label (c);
            c = mid.column ();
            role = mid.role ();
        }
    }
    return m_->baseModel()->index (r, c).data (role);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QVariant GroupSubModel::headerData (
        int section, Qt::Orientation orientation, int role ) const
{
    if (orientation == Qt::Vertical) {
        return QString::number (section+1);
    }
    return m_->baseModel()->headerData (section, orientation, role);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 *
 * @param row a zero-based index in the list of items this model manages
 * @return -1 if the \b row is outside valid range, a row in base model otherwise
 */
int GroupSubModel::mapRowToBaseModel (int row) const
{
    int r;
    int r_max = map_.count();
    if ((row < 0) || (row >= r_max)) {
        return -1;
    } else if (m_->sortingDirection() == Qt::AscendingOrder) {
        r = map_.at (row);
    } else {
        r = map_.at (r_max - row - 1);
    }
    return r;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int GroupSubModel::listIndex () const
{
    if (m_->sortingDirection() == Qt::AscendingOrder) {
        return list_index_;
    } else {
        return m_->groupCount() - list_index_;
    }

}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupSubModel::signalReset()
{
    beginResetModel();
    endResetModel();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupSubModel::baseModelDataChange (
        int index_in_group, const QVector<int> &roles)
{
    QModelIndex idx = index (index_in_group, 0);
    emit dataChanged (idx, idx, roles);
}
/* ========================================================================= */

void GroupSubModel::anchorVtable () const {}
