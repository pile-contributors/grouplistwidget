/**
 * @file groupmodel.cc
 * @brief Definitions for GroupModel class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "groupmodel.h"
#include "groupsubmodel.h"
#include "grouplistwidget-private.h"
#include <assert.h>
#include <QAbstractItemModel>
#include <QUrl>
#include <QRegExp>
#include <QRegularExpression>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QPoint>
#include <QPointF>
#include <QSize>
#include <QSizeF>
#include <QLine>
#include <QLineF>
#include <QRect>
#include <QRectF>
#include <QPixmap>
#include <QCoreApplication>
#include <QMap>


/**
 * @class GroupModel
 *
 * This class provides an intermediate layer between the model provided by
 * the user and the actual widget. The class may be sub-classed to get
 * full control of the internal workings. However, most of the times, it
 * is enough to provide a "customizer" that formats the data accordingly.
 *
 * Internally, the groups are always sorted in ascending order.
 * The group() method inspects the desired groupingDirection()
 * and returns the result accordingly.
 *
 * A number of signals are used to communicate with the widgets
 * presenting the data:
 * - modelAboutToBeReset() and modelReset() are raised both
 * when the base model changes and when the grouping column changes;
 * the widgets must respond by reconstructing the entire view.
 * - groupingChanged() informs the widget that the order of the grouping
 * should change; the view may simply reorder the groups without
 * reconstructing everything.
 * - sortingChanged() is raised when either the direction or
 * the column used fr sorting changes; as the underlying
 * GroupSubModel also raises signals for associated list
 * widgets, the top level widget does not use this signal.
 */

/* ------------------------------------------------------------------------- */
/**
 * Default constructor creates a usable instance that can be readily used
 * with a list widget. The user still needs to install a model for
 * the widget to present anything useful.
 * Alternatively, the user may provide a model to the constructor that will
 * be installed by the constructor.
 *
 * The instance owns the QAbstractItemModel that is destroyed in the
 * destructor. To avoid the destructor the user may choose to
 * takeUnderModel() before the destructor kicks in.
 */
GroupModel::GroupModel (QAbstractItemModel * model, QObject * parent) :
    QObject(parent),
    m_base_(model),
    pixmap_(-1, Qt::DecorationRole),
    group_(-1, Qt::EditRole),
    group_label_role_(Qt::DisplayRole),
    group_dir_(Qt::AscendingOrder),
    group_func_(defaultCompare),
    sort_(-1, Qt::EditRole),
    sort_dir_(Qt::AscendingOrder),
    sort_func_(defaultCompare),
    user_data_(),
    groups_(),
    supress_signals_(false),
    additional_labels_()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    additional_labels_.append (ModelId(0, Qt::DisplayRole));
    assert(additional_labels_.at(0).column() == 0);
    assert(additional_labels_.at(0).role() == Qt::DisplayRole);
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 */
GroupModel::~GroupModel()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    uninstallBaseModel();
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This instance takes ownership of provided pointer. Currently installed
 * model will be deleted by this method. To avoid this use takeBaseModel()
 * prior to calling this method.
 *
 * @param model The model that provides the data to sort, group and present.
 */
void GroupModel::setBaseModel (
        QAbstractItemModel *model, int grouping_col, int sorting_col,
        Qt::SortOrder group_dir, Qt::SortOrder sort_dir )
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    if (baseModel () == model) {
        GROUPLISTWIDGET_DEBUGM("Attempt to install same model "
                               "twice blocked.\n");
        return;
    }

    emit modelAboutToBeReset();
    supress_signals_ = true;

    installBaseModel (model);

    group_.setColumn (grouping_col);
    sort_.setColumn (sorting_col);
    group_dir_ = group_dir;
    sort_dir_ = sort_dir;

    if (baseModel () != NULL) {
        if (group_.column () != -1)
            buildAllGroups ();
        else
            buildNoGroupingGroup ();
    }

    supress_signals_ = false;
    emit modelReset();

    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * @return installed model (may be NULL).
 */
QAbstractItemModel *GroupModel::baseModel() const
{
    return m_base_;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The ownership of the pointer is transferred to the caller.
 * The model is uninstalled and a NULL model is installed.
 *
 * @return previously installed model
 */
QAbstractItemModel * GroupModel::takeBaseModel ()
{
    GROUPLISTWIDGET_TRACE_ENTRY;

    QAbstractItemModel * tmp = baseModel ();
    uninstallBaseModel (false);

    GROUPLISTWIDGET_TRACE_EXIT;
    return tmp;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::installBaseModel (QAbstractItemModel * value)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    uninstallBaseModel ();

    if (value != NULL) {
        connect (value, &QAbstractItemModel::modelAboutToBeReset,
                 this, &GroupModel::modelAboutToBeReset);
        connect (value, &QAbstractItemModel::modelReset,
                 this, &GroupModel::modelReset);
        connect (value, &QAbstractItemModel::dataChanged,
                 this, &GroupModel::baseModelDataChange);
//        connect (value, &QAbstractItemModel::rowsRemoved,
//                 this, &GroupModel::baseModelRowsRemoved);
    }

    m_base_ = value;
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::uninstallBaseModel (bool do_delete)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    clearAllGroups ();
    if (m_base_ != NULL) {
        disconnect (m_base_, &QAbstractItemModel::modelAboutToBeReset,
                    this, &GroupModel::modelAboutToBeReset);
        disconnect (m_base_, &QAbstractItemModel::modelReset,
                    this, &GroupModel::modelReset);
        disconnect (m_base_, &QAbstractItemModel::dataChanged,
                    this, &GroupModel::baseModelDataChange);
//        disconnect (m_base_, &QAbstractItemModel::rowsRemoved,
//                    this, &GroupModel::baseModelRowsRemoved);

        if (do_delete)
            delete m_base_;
    }

    m_base_ = NULL;
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::baseModelDataChange (
        const QModelIndex &topLeft, const QModelIndex &bottomRight,
        const QVector<int> &roles)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    if (groups_.count() > 0) {
        int i = qMin (topLeft.row(), bottomRight.row());
        int i_max = i + qAbs (topLeft.row() - bottomRight.row()) + 1;
        for (; i < i_max; ++i) {
            int index_in_group = -1;
            GroupSubModel * grp = groupForRow (i, &index_in_group);
            if (grp == NULL) {
                GROUPLISTWIDGET_DEBUGM(
                            "Received word that row %d changed in "
                            "base model but it was not found in groups\n",
                            i);
            } else {
                grp->baseModelDataChange (
                            index_in_group, roles);
            }
        }
    }
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::baseModelRowsRemoved (
        const QModelIndex & /*parent*/, int first, int last)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
#if 0
    if (groups_.count() > 0) {
        QMap<int, QList<int> > affected;
        for (int i = first; i <= last; ++i) {
            int index_in_group = -1;
            GroupSubModel * grp = groupForRow (i, &index_in_group);
            if (grp == NULL) {
                GROUPLISTWIDGET_DEBUGM(
                            "Received word that row %d was removed in "
                            "base model but it was not found in groups\n",
                            i);
            } else {
                QList<int> rem_lst = affected.value (
                            grp->listIndex(), QList<int>());
                rem_lst.append (index_in_group);
                affected[grp->listIndex()] = rem_lst;
            }
        }

        foreach(int k, affected.keys ()) {
            GroupSubModel * grp = groups_.at (k);
            QList<int> rem_lst = affected.value (k);
            // sort the list of rows to remove from largest to smallest
            qSort (rem_lst.begin(), rem_lst.end(), qGreater<int>());

            // make only necessary calls (join intervals in a single group)
            int count = 0;
            int to_rem = -11;
            int i_max = rem_lst.count();
            for (int i = 0; i < i_max; ++i) {
                int idx = rem_lst.at (i);
                if (idx == to_rem - 1) {
                    // join with previous
                    --to_rem;
                    count++;
                } else {
                    if (count > 0) {
                        // if not first one
                        grp->removeRows (to_rem, count);
                    }
                    count = 1;
                    to_rem = idx;
                }
            }
            if (count > 0) {
                grp->removeRows (to_rem, count);
            }
        }
    }
#else

#endif
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * There is no internal reverse mapping from rows to groups, so this is an
 * expensive method as it has to search on average half the rows.
 *
 * @param base_row The 0 based index in the base model.
 * @param index_in_group if found, this will hold the index inside the group
 * of the item.
 * @return the sub-model or NULL
 */
GroupSubModel * GroupModel::groupForRow (
        int base_row, int * index_in_group)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    foreach(GroupSubModel * subm, groups_) {
        int idx = subm->mapping ().indexOf (base_row);
        if (idx != -1) {
            if (index_in_group != NULL) {
                if (sort_dir_ == Qt::AscendingOrder) {
                    *index_in_group = idx;
                } else {
                    *index_in_group = subm->rowCount() - idx - 1;
                }
            }
            return subm;
        }
    }
    GROUPLISTWIDGET_TRACE_EXIT;
    return NULL;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::setPixmapColumn (int column)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    pixmap_.setColumn (column);
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int GroupModel::pixmapColumn () const
{
    return pixmap_.column ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::setPixmapRole (Qt::ItemDataRole role)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    pixmap_.setRole (role);
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
Qt::ItemDataRole GroupModel::pixmapRole () const
{
    return pixmap_.role ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The result type depends on the underlying model.
 *
 * @param row Indicates the item using its zero-based index.
 */
QPixmap GroupModel::pixmap (int row) const
{
    if (pixmap_.column () == -1)
        return QPixmap();
    if (baseModel () == NULL)
        return QPixmap();
    return qvariant_cast<QPixmap>(
                baseModel ()->index(row, pixmap_.column ()).data (pixmap_.role ()));
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int GroupModel::count() const
{
    if (baseModel () == NULL)
        return 0;
    else
        return baseModel ()->rowCount ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool GroupModel::remove (int row)
{
    bool b_ret = false;
    if (baseModel () == NULL) {
        GROUPLISTWIDGET_DEBUGM("No base model to remove from\n");
    } else {
        int row_max = baseModel()->rowCount ();
        if ((row < 0) || (row >= row_max)) {
            GROUPLISTWIDGET_DEBUGM("Row %d is outside valid range "
                                   "[0..%d)\n",
                                   row, row_max);
        } else {
            b_ret = baseModel ()->removeRow (row);
            if (b_ret) {
                //emit modelAboutToBeReset ();
                //emit modelReset ();
                regroup ();
            }
        }
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The column may be -1 to indicate that no grouping should be performed
 * or an index inside the valid range for columns in base model.
 *
 * Please note that, if the old column is the same as the new one,
 * nothing happens and the result is true.
 *
 * @param column zero-based index of the column inside base model
 * @return true if the grouping column is the one in the argument.
 */
bool GroupModel::setGroupingColumn (int column)
{
    bool b_ret = false;
    GROUPLISTWIDGET_TRACE_ENTRY;
    for (;;) {

        if (column == -1) {
            // if no grouping is requested we're good to go

            // be lazy
            if (group_.column () == column) {
                b_ret = true;
                break;
            }

            if (!supress_signals_)
                emit modelAboutToBeReset ();
            if (baseModel() != NULL) {
                clearAllGroups ();
                group_.setColumn (column);
                buildNoGroupingGroup ();
            } else {
                group_.setColumn (column);
            }
            if (!supress_signals_)
                emit modelReset ();

            b_ret = true;
            break;
        }

        // make sure provided index is valid
        if (baseModel() != NULL) {
            int col_max = baseModel()->columnCount ();
            if ((column < 0) || (column >= col_max)) {
                GROUPLISTWIDGET_DEBUGM("Grouping column %d is outside valid range "
                                       "[0..%d)\n",
                                       column, col_max);
                break;
            }
        }
        /*if (column == sort_.column ()) {
            if (group_.role () == sort_.role ()) {
                GROUPLISTWIDGET_DEBUGM("Grouping column can't be the same as "
                                       "sorting column (%d)\n",
                                       column);
                return false;
            }
        }*/
        if (column == pixmap_.column ()) {
            if (group_.role () == pixmap_.role ()) {
                GROUPLISTWIDGET_DEBUGM("Grouping column can't be the same as "
                                       "pixmap column (%d)\n",
                                       column);
                break;
            }
        }

        // be lazy
        if (group_.column () == column) {
            b_ret = true;
            break;
        }

        if (!supress_signals_)
            emit modelAboutToBeReset ();
        if (baseModel () != NULL) {
            clearAllGroups ();
            group_.setColumn (column);
            buildAllGroups ();
        } else {
            group_.setColumn (column);
        }
        if (!supress_signals_)
            emit modelReset ();

        b_ret = true;
        break;
    }
    return b_ret;
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
GroupSubModel * GroupModel::group (int idx) const
{
    if ((idx < 0) || (idx >= groups_.count())) {
        GROUPLISTWIDGET_DEBUGM("Group index %d is outside valid range "
                               "[0..%d)\n",
                               idx, groups_.count());
        return NULL;
    }
    if (group_dir_ == Qt::AscendingOrder) {
        return groups_.at (idx);
    } else {
        return groups_.at (groups_.count() - idx - 1);
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The column may be -1 to indicate that no sorting should be performed
 * or an index inside the valid range for columns in base model.
 *
 * The column may not be the same as the grouping column or the pixmap column.
 *
 * Please note that, if the old column is the same as the new one,
 * nothing happens and the result is true.
 *
 * @param column zero-based index of the column inside base model
 * @return true if the sorting column is the one in the argument.
 */
bool GroupModel::setSortingColumn (int column)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    bool b_ret = false;
    for (;;) {

        if (column == -1) {
            // if no sorting is requested we're good to go

            // be lazy
            if (sort_.column () == column) {
                b_ret = true;
                break;
            }

            sort_.setColumn (column);
            if (baseModel() != NULL)
                performUnsorting ();

            if (!supress_signals_)
                emit sortingChanged (sort_.column (), sort_dir_);

            b_ret = true;
            break;
        }

        // make sure provided index is valid
        if (baseModel () != NULL) {
            if ((column < 0) || (column >= baseModel()->columnCount ())) {
                GROUPLISTWIDGET_DEBUGM("Sorting column %d is outside valid range "
                                       "[0..%d)\n",
                                       column, baseModel()->columnCount ());
                return false;
            }
        }
        /*if (column == group_.column ()) {
            if (group_.role () == sort_.role ()) {
                GROUPLISTWIDGET_DEBUGM("Sorting column can't be the same as "
                                       "grouping column (%d)\n",
                                       column);
                return false;
            }
        }*/
        if (column == pixmap_.column ()) {
            if (sort_.role () == pixmap_.role ()) {
                GROUPLISTWIDGET_DEBUGM("Sorting column can't be the same as "
                                       "pixmap column (%d)\n",
                                       column);
                return false;
            }
        }

        // be lazy
        if (sort_.column () == column) {
            b_ret = true;
            break;
        }

        sort_.setColumn (column);
        if (baseModel() != NULL)
            performSorting ();
        if (!supress_signals_)
            emit sortingChanged (sort_.column (), sort_dir_);

        b_ret = true;
        break;
    }
    GROUPLISTWIDGET_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::setSortingDirection(Qt::SortOrder value)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    sort_dir_ = value;
    if (!supress_signals_) {
        foreach (GroupSubModel * subm, groups_) {
            subm->signalReset ();
        }
        emit sortingChanged (sort_.column (), sort_dir_);
    }
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::addLabels (
        const QList<int> & col_lst, Qt::ItemDataRole role)
{
    emit modelAboutToBeReset ();
    foreach (int i, col_lst) {
        additional_labels_.append (ModelId (i, role));
    }
    emit modelReset ();
    // resetAllSubGroups ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * @param item Indicates the item using its zero-based index.
 * @param pos Indicates which labels is requested (0 - main label).
 */
QString GroupModel::itemLabel (int item, int pos) const
{
    QString result;
    for (;;) {
        ModelId mid = label (pos);
        if (!mid.isValid()) {
            GROUPLISTWIDGET_DEBUGM(
                        "Can't retrieve label for invalid position %d.\n",
                        pos);
            break;
        }

        result = baseModel()->index (
                    item, mid.column())
                .data (mid.role()).toString();

        break;
    }
    return result;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The method asserts that there is a base model installed and that
 * the grouping column has a value inside valid range.
 */
void GroupModel::buildAllGroups ()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    Q_ASSERT (baseModel () != NULL);
    Q_ASSERT (group_.column () >= 0);
    Q_ASSERT (group_.column () < baseModel ()->columnCount ());
    int i_max = baseModel ()->rowCount();
    int group_index = 0;

#   define NEW_GROUP \
    GroupSubModel * newm = new GroupSubModel ( \
                this, iter_data, midx.data(group_label_role_).toString()); \
    newm->appendRecord (i);

    // go through all records in the base model
    for (int i = 0; i < i_max; ++i) {
        QModelIndex midx = baseModel ()->index (i, group_.column ());
        QVariant iter_data = midx.data (group_.role ());

        bool b_found = false;
        group_index = 0;
        foreach(GroupSubModel * subm, groups_) {
            ComparisonReslt res = group_func_(
                        this, group_.column (), iter_data,
                        subm->groupKey ());
            switch (res) {
            case Equal: {
                subm->insertSortedRecord (i);
                b_found = true;
                break; }
            case Smaller: {
                NEW_GROUP;
                groups_.insert (group_index, newm);
                b_found = true;
                break; }
            case Larger: {
                break; }
            }
            if (b_found)
                break;
            ++group_index;
        }
        if (!b_found) {
            NEW_GROUP;
            groups_.append (newm);
        }
    }

    // let every group know their place
    group_index = 0;
    foreach(GroupSubModel * subm, groups_) {
        subm->setListIndex (group_index);
        ++group_index;
    }

    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The method asserts that there is a base model installed.
 */
void GroupModel::buildNoGroupingGroup ()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    GroupSubModel * newm = new GroupSubModel (
                this, QVariant(),
                tr("(ungrouped)"));

    // go through all records in the base model
    int i_max = baseModel ()->rowCount();
    for (int i = 0; i < i_max; ++i) {
        newm->insertSortedRecord (i);
    }

    groups_.append (newm);
    newm->setListIndex (0);
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The state at the end of this method is not appropriate for run-time.
 * It should be followed by either buildNoGroupingGroup() or
 * buildAllGroups().
 */
void GroupModel::clearAllGroups ()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    qDeleteAll (groups_);
    groups_.clear ();
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The method asserts that there is a base model installed.
 */
void GroupModel::performSorting ()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    // go through all groups
    foreach (GroupSubModel * subm, groups_) {
        subm->performSorting ();
    }
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The method asserts that there is a base model installed.
 */
void GroupModel::performUnsorting ()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    // go through all groups
    foreach (GroupSubModel * subm, groups_) {
        subm->performUnsorting ();
    }
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Default implementation simply excludes the grouping and pixmap columns
 * and returns everything else.
 * @return the index in the base model of the columns where sorting can be applied.
 */
QList<int> GroupModel::sortingColumns () const
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    QList<int> result;
    int i_max = baseModel ()->columnCount();
    result.reserve (i_max - 2);
    for (int i = 0; i < i_max; ++i) {
        if ((i == pixmap_.column ()) && (sort_.role () == pixmap_.role ()))
            continue;
        /*if (i == group_.column ())
            continue;*/
        result.append (i);
    }

    GROUPLISTWIDGET_TRACE_EXIT;
    return result;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QStringList GroupModel::columnLabels (
        const QList<int> & scols, int * idx_crt, int highlite) const
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    QStringList result;
    int idx_crt_in_slist = -1;
    result.reserve (scols.count());

    int i = 0;
    foreach(int col, scols) {
        result.append(
                    baseModel ()->headerData (
                        col, Qt::Horizontal, Qt::DisplayRole).toString());

        if (col == highlite)
            idx_crt_in_slist = i;
        ++i;
    }

    if (idx_crt != NULL)
        *idx_crt = idx_crt_in_slist;

    GROUPLISTWIDGET_TRACE_EXIT;
    return result;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QStringList GroupModel::sortingColumnLabels (int * idx_crt) const
{
    return columnLabels (sortingColumns (), idx_crt, sort_.column ());
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QList<int> GroupModel::groupingColumns () const
{
    QList<int> result;
    int i_max = baseModel ()->columnCount();
    result.reserve (i_max - 2);
    for (int i = 0; i < i_max; ++i) {
        if ((i == pixmap_.column ()) && (group_.role () == pixmap_.role ()))
            continue;
        /*if (i == sort_.column ())
            continue;*/
        result.append (i);
    }
    return result;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QStringList GroupModel::groupingColumnLabels (int * idx_crt) const
{
    return columnLabels (groupingColumns (), idx_crt, group_.column ());
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::resetAllSubGroups ()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    foreach (GroupSubModel * subm, groups_) {
        subm->signalReset ();
    }
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
ModelId GroupModel::label (int idx) const
{
    int idx_max = additional_labels_.length ();
    if ((idx < 0) || (idx > additional_labels_.length ())) {
        GROUPLISTWIDGET_DEBUGM(
                    "Label index %d outside valid range [0; %d).\n",
                    idx, idx_max);
        return ModelId ();
    } else {
        return additional_labels_.at (idx);
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::regroup ()
{
    if (!supress_signals_)
        emit modelAboutToBeReset ();
    clearAllGroups();
    if (baseModel () != NULL) {
        if (group_.column () != -1)
            buildAllGroups ();
        else
            buildNoGroupingGroup ();
    }
    if (!supress_signals_)
        emit modelReset ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This default implementation expects the two types to be the same.
 * Some of the types will be handled and others will throw an
 * assertion error. Check the code to see supported types.
 *
 * @param model this instance
 * @param column the column where the values belong
 * @param v1 the value for first row
 * @param row1 first row
 * @param v2 the value for second row
 * @param row2 second row
 * @return the result of the comparison
 */
GroupModel::ComparisonReslt GroupModel::defaultCompare (
        GroupModel * /*model*/, int /*column*/,
        const QVariant &v1, const QVariant &v2)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    if (v1.isNull() && v2.isNull()) {
        return Equal;
    } else if (v1.isNull()) {
        return Smaller;
    } else if (v2.isNull()) {
        return Larger;
    } else if (v1.type() == v2.type()) {
        switch (v1.type()) {
        case QVariant::Bool: {
            bool vv1 = v1.toBool(); bool vv2 = v2.toBool();
            if (vv1 == vv2) return Equal;
            return vv1 ? Larger : Smaller;
        }
        case QVariant::Int: {
            int vv1 = v1.toInt (); int vv2 = v2.toInt();
            if (vv1 == vv2) return Equal;
            return vv1 > vv2 ? Larger : Smaller;
        }
        case QVariant::UInt: {
            uint vv1 = v1.toUInt (); uint vv2 = v2.toUInt();
            if (vv1 == vv2) return Equal;
            return vv1 > vv2 ? Larger : Smaller;
        }
        case QVariant::LongLong: {
            qlonglong vv1 = v1.toLongLong (); qlonglong vv2 = v2.toLongLong();
            if (vv1 == vv2) return Equal;
            return vv1 > vv2 ? Larger : Smaller;
        }
        case QVariant::ULongLong: {
            qulonglong vv1 = v1.toULongLong (); qulonglong vv2 = v2.toULongLong();
            if (vv1 == vv2) return Equal;
            return vv1 > vv2 ? Larger : Smaller;
        }
        case QVariant::Double: {
            double vv1 = v1.toDouble (); double vv2 = v2.toDouble();
            if (qFuzzyCompare (vv1, vv2)) return Equal;
            return vv1 > vv2 ? Larger : Smaller;
        }
        case QVariant::Char: {
            QChar vv1 = v1.toChar (); QChar vv2 = v2.toChar();
            if (vv1 == vv2) return Equal;
            return vv1 > vv2 ? Larger : Smaller;
        }
        case QVariant::Url:
        case QVariant::RegExp:
        case QVariant::RegularExpression:
        case QVariant::Hash:
        case QVariant::Uuid:
        case QVariant::String: {
            int res = QString::compare(
                        v1.toString(), v2.toString(), Qt::CaseInsensitive);
            if (res == 0) return Equal;
            return res > 0 ? Larger : Smaller;
        }
        case QVariant::Date: {
            QDate vv1 = v1.toDate (); QDate vv2 = v2.toDate();
            if (vv1 == vv2) return Equal;
            return vv1 > vv2 ? Larger : Smaller;
        }
        case QVariant::Time: {
            QTime vv1 = v1.toTime (); QTime vv2 = v2.toTime();
            if (vv1 == vv2) return Equal;
            return vv1 > vv2 ? Larger : Smaller;
        }
        case QVariant::DateTime: {
            QDateTime vv1 = v1.toDateTime (); QDateTime vv2 = v2.toDateTime ();
            if (vv1 == vv2) return Equal;
            return vv1 > vv2 ? Larger : Smaller;
        }
        case QVariant::Size: {
            QSize vv1 = v1.toSize (); QSize vv2 = v2.toSize ();
            if (vv1.width() > vv2.width()) return Larger;
            if (vv1.width() < vv2.width()) return Smaller;
            if (vv1.height() > vv2.height()) return Larger;
            if (vv1.height() < vv2.height()) return Smaller;
            return Equal;
        }
        case QVariant::SizeF: {
            QSizeF vv1 = v1.toSizeF (); QSizeF vv2 = v2.toSizeF ();
            if (vv1.width() > vv2.width()) return Larger;
            if (vv1.width() < vv2.width()) return Smaller;
            if (vv1.height() > vv2.height()) return Larger;
            if (vv1.height() < vv2.height()) return Smaller;
            return Equal;
        }
        case QVariant::Point: {
            QPoint vv1 = v1.toPoint (); QPoint vv2 = v2.toPoint ();
            if (vv1.x() > vv2.x()) return Larger;
            if (vv1.x() < vv2.x()) return Smaller;
            if (vv1.y() > vv2.y()) return Larger;
            if (vv1.y() < vv2.y()) return Smaller;
            return Equal;
        }
        case QVariant::PointF: {
            QPointF vv1 = v1.toPointF (); QPointF vv2 = v2.toPointF ();
            if (vv1.x() > vv2.x()) return Larger;
            if (vv1.x() < vv2.x()) return Smaller;
            if (vv1.y() > vv2.y()) return Larger;
            if (vv1.y() < vv2.y()) return Smaller;
            return Equal;
        }
        default: {
            GROUPLISTWIDGET_DEBUGM("Cannot compare %s (not supported)\n",
                                   v1.typeName ());
            break;}
        }
    } else {
        GROUPLISTWIDGET_DEBUGM("Cannot compare %s with %s\n",
                               v1.typeName (), v2.typeName ());
    }
    Q_ASSERT(false);
    GROUPLISTWIDGET_TRACE_EXIT;
    return Equal;
}
/* ========================================================================= */

void GroupModel::anchorVtable () const {}
