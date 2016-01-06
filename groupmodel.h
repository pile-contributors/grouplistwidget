/**
 * @file groupmodel.h
 * @brief Declarations for GroupModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */
#ifndef GUARD_GROUPMODEL_H_INCLUDE
#define GUARD_GROUPMODEL_H_INCLUDE

#include <grouplistwidget/grouplistwidget-config.h>
#include <QVariant>
#include <QList>
#include <QObject>
#include <QVector>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
QT_END_NAMESPACE

class GroupSubModel;

//! Groups the column and the role for a specific task.
class ModelId : private QPair<int,Qt::ItemDataRole> {
public:
    ModelId () : QPair<int,Qt::ItemDataRole>(-1, Qt::UserRole) {}
    ModelId (int column, Qt::ItemDataRole role) : QPair<int,Qt::ItemDataRole>(column, role) {}
    int column () const { return first; }
    void setColumn (int value) { first = value; }
    Qt::ItemDataRole role () const { return second; }
    void setRole (Qt::ItemDataRole value) { second = value; }
};

//! A model that is to be installed into a GroupListWidget.
class GROUPLISTWIDGET_EXPORT GroupModel : public QObject {
    Q_OBJECT
public:


    enum { BaseColRole = Qt::UserRole + 1000 };

    //! The result of comparing two values.
    enum ComparisionReslt {
        Equal = 0,
        Smaller = -1,
        Larger = 1
    };

    //! Compare two QVariants.
    typedef ComparisionReslt (*Compare) (
            GroupModel * model,
            int column,
            const QVariant & v1,
            const QVariant & v2);

    /*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */

    //! Default constructor.
    GroupModel (
            QAbstractItemModel *model = NULL,
            QObject * parent = NULL);

    //! Destructor.
    virtual ~GroupModel();

    //! Sets the user model.
    virtual void
    setBaseModel (
            QAbstractItemModel * model,
            int grouping_col = -1,
            int sorting_col = -1,
            Qt::SortOrder group_dir = Qt::AscendingOrder,
            Qt::SortOrder sort_dir = Qt::AscendingOrder);

    //! Retrieve user model.
    virtual QAbstractItemModel *
    baseModel () const;

    //! Take ownership of user model.
    QAbstractItemModel *
    takeBaseModel ();

    //! Sets the column in base model that provides the image for the icon.
    virtual void
    setPixmapColumn (
            int column);

    //! Retrieve the column in base model that provides the image for the icon.
    virtual int
    pixmapColumn () const;

    //! Sets the role in base model that provides the image for the icon.
    virtual void
    setPixmapRole (
            Qt::ItemDataRole role);

    //! Retrieve the role in base model that provides the image for the icon.
    virtual Qt::ItemDataRole
    pixmapRole () const;

    //! Retrieve the pixmap for a particular row.
    virtual QPixmap
    pixmap (
            int row) const;


    //! Sets some opaque data useful for the user.
    virtual void
    setUserData (
            const QVariant & value) {
        user_data_ = value;
    }

    //! Retrieve the opaque data useful for the user.
    virtual const QVariant &
    userData () const {
        return user_data_;
    }


signals:

    //! Before the actual changes are implemented.
    void
    modelAboutToBeReset ();

    //! After the model was updated.
    void
    modelReset ();

public:

    //! Default implementation for comparision function
    static ComparisionReslt
    defaultCompare (
            GroupModel * model,
            int column,
            const QVariant & v1,
            const QVariant & v2);

protected:

    //! Implementation used by sortingColumnLabels() and groupingColumnLabels ()
    virtual QStringList
    columnLabels (
            const QList<int> & scols,
            int * idx_crt,
            int highlite) const;

    //! Reset all sub-groups models.
    void
    resetAllSubGroups ();


    /*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
    /** @name Labels
     * The model can present one or more labels for each
     * item. The methods in this group allow the user
     * to manage the list of column:role pairs for each
     * label.
     *
     * @note The standard, unenhanced view will only show
     * the first label and will always use DisplayRole.
     * Use GroupListDelegate to show all these labels.
     */
    ///@{

public:


    //! Column and role for main label.
    const ModelId &
    label () const {
        Q_ASSERT(additional_labels_.length () > 0);
        return additional_labels_.at (0);
    }

    //! Number of labels.
    int
    labelCount () const {
        return additional_labels_.count ();
    }

    //! Column and role for a label.
    ModelId
    label (int idx) const {
        if ((idx < 0) || (idx > additional_labels_.length ())) {
            return ModelId ();
        } else {
            return additional_labels_.at (idx);
        }
    }

    //! Add the column and role for an additional label to be presented to the user.
    void
    addLabel (
            const ModelId & value) {
        additional_labels_.append (value);
        resetAllSubGroups ();
    }

    //! Add the column and role for a label to be presented to the user.
    void
    addLabel (
            int column, Qt::ItemDataRole role = Qt::DisplayRole) {
        addLabel (ModelId (column, role));
    }

    //! Set the column and role for main label.
    void
    setLabel (
            const ModelId & value) {
        Q_ASSERT(additional_labels_.length () > 0);
        additional_labels_[0] = value;
        resetAllSubGroups ();
    }

    //! Set the column and role for a label.
    void
    setLabel (
            const ModelId & value,
            int idx) {
        if ((idx < 0) || (idx > additional_labels_.length ())) {
            return;
        } else {
            additional_labels_[idx] = value;
            resetAllSubGroups ();
        }
    }

    //! Set the column and role for a label.
    void
    setLabel (
            int column,
            Qt::ItemDataRole role = Qt::DisplayRole,
            int idx = 0) {
        setLabel (ModelId (column, role), idx);
    }

    ///@}
    /*  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */


    /*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
    /** @name Grouping
     * All methods related to grouping are gathered
     * in this section.
     */
    ///@{

public:

    //! Retrieve the column in base model that decides grouping.
    virtual int
    groupingColumn () const {
        return group_.column ();
    }

    //! Tell if we're grouping the records or not.
    virtual int
    isGrouping () const {
        return group_.column() != -1;
    }

    //! Sets the role in base model that decides grouping.
    virtual void
    setGroupingRole (
            Qt::ItemDataRole role) {
        group_.setRole (role);
    }

    //! Retrieve the role in base model that decides grouping.
    virtual Qt::ItemDataRole
    groupingRole () const {
        return group_.role ();
    }

    //! Sets the role in base model that provides group label.
    virtual void
    setGroupLabelRole (
            Qt::ItemDataRole role) {
        group_label_role_ = role;
    }

    //! Retrieve the role in base model that provides group label.
    virtual Qt::ItemDataRole
    groupLabelRole () const {
        return group_label_role_;
    }

    //! Retrieve the direction of grouping.
    virtual Qt::SortOrder
    groupingDirection () const {
        return group_dir_;
    }

    //! Sets the function used to assign records to groups.
    virtual void
    setGroupingFunc (
            Compare value) {
        if (value == NULL)
            value = defaultCompare;
        group_func_ = value;
    }

    //! Retrieve the function used to assign records to groups.
    virtual Compare
    groupingFunc () const {
        return group_func_;
    }

    //! Get the group at a particular index.
    virtual GroupSubModel *
    group (
            int idx) const;

    //! Get the number of groups curently defined inside this model.
    virtual int
    groupCount () const {
        return groups_.count();
    }

    //! Retrieve the list of columns where grouping can be applied.
    virtual QList<int>
    groupingColumns () const;

    //! Retrieve the labels for all columns where grouping can be applied.
    virtual QStringList
    groupingColumnLabels (
            int * idx_crt) const;


public slots:

    //! Sets the column in base model that decides grouping.
    virtual bool
    setGroupingColumn (
            int column);

    //! Sets the column in base model that decides grouping by reading columnIndex property of the sender.
    virtual bool
    setGroupingColumnByProperty () {
        return setGroupingColumn (sender()->property("columnIndex").toInt ());
    }

    //! No grouping.
    void
    removeGrouping () {
        setGroupingColumn (-1);
    }

    //! Sets the direction of grouping.
    virtual void
    setGroupingDirection (
            Qt::SortOrder value) {
        group_dir_ = value;
        if (!supress_signals_)
            emit groupingChanged (group_.column(), group_dir_);
    }

    //! Sets the direction of grouping.
    void
    setGroupingAscending () {
        setGroupingDirection (Qt::AscendingOrder);
    }

    //! Sets the direction of grouping.
    void
    setGroupingDescending () {
        setGroupingDirection (Qt::DescendingOrder);
    }

protected:

    //! Constructs all groups.
    virtual void
    buildAllGroups();

    //! Special case when no grouping is enabled.
    virtual void
    buildNoGroupingGroup();

    //! Destroys all groups.
    virtual void
    clearAllGroups ();

signals:

    //! The direction (NOT the column) of the grouping has changed.
    void
    groupingChanged (
            int column,
            Qt::SortOrder);

    ///@}
    /*  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */



    /*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
    /** @name Sorting
     * All methods related to sorting are gathered
     * in this section.
     */
    ///@{

public:

    //! Retrieve the column in base model that decides sorting.
    virtual int
    sortingColumn () const {
        return sort_.column();
    }

    //! Tell if we're sorting the records inside groups or not.
    virtual int
    isSorting () const {
        return sort_.column() != -1;
    }

    //! Sets the role in base model that decides sorting.
    virtual void
    setSortingRole (
            Qt::ItemDataRole role) {
        sort_.setRole (role);
        if (!supress_signals_)
            emit sortingChanged (sort_.column(), sort_dir_);
    }

    //! Retrieve the role in base model that decides sorting.
    virtual Qt::ItemDataRole
    sortingRole () const {
        return sort_.role ();
    }


    //! Retrieve the direction of sorting.
    virtual Qt::SortOrder
    sortingDirection () const {
        return sort_dir_;
    }

    //! Sets the function used to sort records inside groups.
    virtual void
    setSortingFunc (
            Compare value) {
        if (value == NULL)
            value = defaultCompare;
        sort_func_ = value;
    }

    //! Retrieve the function used to sort records inside groups.
    virtual Compare
    sortingFunc () const {
        return sort_func_;
    }

    //! Retrieve the list of columns where sorting can be applied.
    virtual QList<int>
    sortingColumns () const;

    //! Retrieve the labels for all columns where sorting can be applied.
    virtual QStringList
    sortingColumnLabels (
            int * idx_crt) const;

public slots:

    //! Sets the column in base model that decides sorting.
    virtual bool
    setSortingColumn (
            int column);

    //! Sets the column in base model that decides sorting by reading columnIndex property of the sender.
    virtual bool
    setSortingColumnByProperty () {
        return setSortingColumn (sender()->property ("columnIndex").toInt ());
    }

    //! No sorting.
    void
    removeSorting () {
        setSortingColumn (-1);
    }

    //! Sets the direction of sorting.
    virtual void
    setSortingDirection (
            Qt::SortOrder value);

    //! Sets the direction of sorting.
    void
    setSortingAscending () {
        setSortingDirection (Qt::AscendingOrder);
    }

    //! Sets the direction of sorting.
    void
    setSortingDescending () {
        setSortingDirection (Qt::DescendingOrder);
    }

protected:

    //! Sorts the items in all groups.
    virtual void
    performSorting ();

    //! Arranges the items in all groups according to their original order.
    virtual void
    performUnsorting ();

signals:

    //! The column or direction of the sorting has changed.
    void
    sortingChanged (
            int column,
            Qt::SortOrder);

    ///@}
    /*  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */


private slots:

    void
    baseModelDataChange (
            const QModelIndex &topLeft,
            const QModelIndex &bottomRight,
            const QVector<int> &roles = QVector<int>());

private:

    //! Install a base model inside this instance.
    void
    installBaseModel (
            QAbstractItemModel * value);

    //! Uninstall current base model from this instance.
    void
    uninstallBaseModel (
            bool do_delete = true);

    //! Find the group that hosts a base model row; this is slow.
    GroupSubModel *
    groupFromBaseRow (
            int base_row,
            int * index_in_group);


    QAbstractItemModel * m_base_; /**< the user model */

    ModelId pixmap_; /**< the column and role in base model that provides the image for the icon */

    ModelId group_; /**< the column and role in base model that decides grouping */
    Qt::ItemDataRole group_label_role_; /**< the role in base model that provides group label */
    Qt::SortOrder group_dir_; /**< direction of grouping */
    Compare group_func_; /**< the function used for grouping */

    ModelId sort_; /**< the column and role in base model that decides sorting */
    Qt::SortOrder sort_dir_; /**< direction of sorting */
    Compare sort_func_; /**< the function used to sort records inside groups */

    QVariant user_data_; /**< user data */

    QList<GroupSubModel*> groups_; /**< the list of groups */
    bool supress_signals_; /**< do we generate signals or not */

    QList<ModelId> additional_labels_; /**< labels to be presented */

public: virtual void anchorVtable() const;
}; // class GroupModel

#endif // GUARD_GROUPMODEL_H_INCLUDE
