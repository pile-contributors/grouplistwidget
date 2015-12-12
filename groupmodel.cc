/**
 * @file grouplistwidget.cc
 * @brief Definitions for GROUPLISTWIDGET class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "grouplistwidget.h"
#include "groupmodel.h"
#include "groupcustomizer.h"
#include "grouplistwidget-private.h"

#include <QAbstractListModel>

#include <assert.h>

class GrouSubModel : public QAbstractListModel  {
    Q_OBJECT

public:

    enum { BaseColRole = Qt::UserRole + 10 };

    //! Create a new model from a table or view definition and a database.
    GrouSubModel (
        GroupModel * parent,
            const QString & s_name = QString ()) :
        QAbstractListModel (parent),
        rows_(),
        name_(s_name),
        label_()
    {
    }

    Q_DISABLE_COPY(GrouSubModel)

    //! Destructor.
    virtual ~GrouSubModel() {

    }

    //! The parent model.
    GroupModel *
    groupModel () const {
        return static_cast<GroupModel *>(parent ());
    }

    int rowCount (const QModelIndex &parent) const {
        return rows_.count();
    }

    QVariant data (
            const QModelIndex &index, int role) const {
        int r = rows_.value (index.row(), -1);
        if (r == -1) {
            return QVariant ();
        }
        int c = 0;
        if (role >= BaseColRole) {
            c = role - BaseColRole;
            role = Qt::DisplayRole;
        }
        GroupModel * gm = groupModel ();
        return gm->data (gm->index (r, c), role);
    }

    //! Retreive name.
    inline const QString &
    label () const {
        return label_;
    }

    //! Set name.
    inline void
    setLabel (const QString & value) {
        label_ = value;
    }


    //! Retreive name.
    inline const QString &
    name () const {
        return name_;
    }

    //! Set name.
    inline void
    setName (const QString & value) {
        name_ = value;
    }

    //! Assign a column to this model.
    inline void
    assignRow (
            int parent_index) {
        rows_.insert (rows_.count() - 1, parent_index);
    }

private:

    QMap<int,int> rows_;
    QString name_;
    QString label_;
};

#include "groupmodel.moc"


/**
 * @class GroupModel
 *
 * .
 */

/* ------------------------------------------------------------------------- */
/**
 *
 *
 * \param parent the parent QObject
 */
GroupModel::GroupModel(QObject * parent) :
    QSortFilterProxyModel (parent),
    col_grouping_(-1),
    col_image_large_(-1),
    col_image_small_(-1),
    groups_()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    // sourceModelChanged ();
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Detailed description for destructor.
 */
GroupModel::~GroupModel()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    if (cust_ != NULL) {
        delete cust_;
    }
    clearAllGroups ();
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::setSourceModel (QAbstractItemModel *source_model)
{
    clearAllGroups ();
    col_grouping_ = -1;
    col_image_large_ = -1;
    col_image_small_ = -1;

    QSortFilterProxyModel::setSourceModel (source_model);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QAbstractListModel *GroupModel::groupList (int idx) const
{
    if ((idx < 0) || (idx >= groups_.count())) {
        return NULL;
    }
    return groups_.at (idx);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QString GroupModel::groupLabel (int idx) const
{
    if ((idx < 0) || (idx >= groups_.count())) {
        return QString ();
    }
    return groups_.at (idx)->label();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QString GroupModel::groupName (int idx) const
{
    if ((idx < 0) || (idx >= groups_.count())) {
        return QString ();
    }
    return groups_.at (idx)->name();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::clearAllGroups ()
{
    foreach(GrouSubModel * itr, groups_) {
        itr->deleteLater();
    }
    groups_.clear ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
GrouSubModel *GroupModel::subModelByName (const QString &s_name) const
{
    foreach(GrouSubModel * itr, groups_) {
        if (itr->name () == s_name) {
            return itr;
        }
    }
    return NULL;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
int GroupModel::subIndexByName (const QString &s_name) const
{
    int i = 0;
    foreach(GrouSubModel * itr, groups_) {
        if (itr->name () == s_name) {
            return i;
        }
        ++i;
    }
    return -1;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::buildAllGroups ()
{
    assert ((col_grouping_ >= 0) && (col_grouping_ < columnCount ()));
    int i_max = rowCount();
    if (cust_ == NULL) {
        cust_ = new GroupCustomizer (this);
    }

    for (int i = 0; i < i_max; ++i) {
        QModelIndex midx = index (i, col_grouping_);
        QString s_name = cust_->name (midx);
        GrouSubModel * subm = subModelByName (s_name);
        if (subm == NULL) {
            GrouSubModel * subm = new GrouSubModel (this, s_name);
            subm->setLabel (cust_->label (midx, s_name));
        }

        subm->assignRow (i);
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::buildNoGroupingGroup ()
{
    assert (groups_.count() == 0);
    assert (col_grouping_ == -1);
    int i_max = rowCount();
    GrouSubModel * mdl = new GrouSubModel (this);
    for (int i = 0; i < i_max; ++i) {
        mdl->assignRow (i);
    }
    groups_.append (mdl);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool GroupModel::setGroupingCol (int value)
{
    bool b_ret = false;
    for (;;) {
        // make sure provided index is valid
        if ((value < 0) || (value >= columnCount ())) {
            return NULL;
        }
        if (value == col_image_large_) {
            return NULL;
        }
        if (value == col_image_small_) {
            return NULL;
        }

        clearAllGroups ();
        col_grouping_ = value;
        buildAllGroups ();

        b_ret = true;
        break;
    }
    return b_ret;

}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupModel::setCustomizer (GroupCustomizer *value)
{
    if (cust_ != NULL) {
        delete cust_;
    }
    cust_ = value;
}
/* ========================================================================= */

