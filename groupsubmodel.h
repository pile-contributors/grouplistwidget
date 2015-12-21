/**
 * @file groupsubmodel.h
 * @brief Declarations for GroupSubModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */
#ifndef GUARD_GROUPSUBMODEL_H_INCLUDE
#define GUARD_GROUPSUBMODEL_H_INCLUDE

#include <grouplistwidget/grouplistwidget-config.h>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QAbstractListModel>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
QT_END_NAMESPACE

class GroupModel;

//! A model representing a group that is used by the embedded lists.
class GROUPLISTWIDGET_EXPORT GroupSubModel : public QAbstractListModel {
    Q_OBJECT

    friend class GroupModel;
public:

    //! Default constructor.
    GroupSubModel (
            GroupModel *model = NULL,
            const QVariant & key = QVariant(),
            const QString & lbl = QString());

    //! Destructor.
    virtual ~GroupSubModel();

    //! Retreive the index of this model inside the main model.
    int
    listIndex () const;

    //! Sets the parent model.
    void
    setParentModel (
            GroupModel * model) {
        m_ = model;
    }

    //! Retreive the parent model.
    GroupModel *
    parentModel () const {
        return m_;
    }

    //! Insert a new record at the end of the list.
    void
    appendRecord (
            int original_row) {
        map_.append (original_row);
    }

    //! Insert a new record in the proper place.
    void
    insertSortedRecord (
            int row);


    //! Sets the key for the grouping algorithm.
    virtual void
    setGroupKey (
            const QVariant & value) {
        key_ = value;
    }

    //! Retreive the key for the grouping algorithm.
    virtual const QVariant &
    groupKey () const {
        return key_;
    }

    //! Sets the user visible label for this group.
    virtual void
    setLabel (
            const QString & value) {
        s_label_ = value;
    }

    //! Retreive the user visible label for this group.
    virtual const QString &
    label () const {
        return s_label_;
    }

    //! Sort internal rows acording to the rules of the parent.
    void
    performSorting ();

    //! Sort internal rows acording to their values.
    void
    performUnsorting ();

    int
    rowCount (
            const QModelIndex &parent = QModelIndex()) const
    {
        return map_.count();
    }

    QVariant
    data (
            const QModelIndex &index,
            int role) const;

    QVariant
    headerData (
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;

    //! Retrieve the row in base model given the row in this model.
    int
    mapRowToBaseModel (
            int row) const;

protected:

    void
    signalReset ();

    void
    setListIndex (
            int value) {
        list_index_ = value;
    }

private:
    GroupModel * m_; /**< the parent model */
    QList<int> map_; /**< maps rows in this model to rows in base model */
    QVariant key_; /**< the key for the grouping algorithm */
    QString s_label_; /**< the user visible label for this group */
    int list_index_; /**< index of this model within main model */
}; // class GroupSubModel

#endif // GUARD_GROUPSUBMODEL_H_INCLUDE
