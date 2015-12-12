/**
 * @file dbmodel.h
 * @brief Declarations for DbModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_GROUPMODEL_H_INCLUDE
#define GUARD_GROUPMODEL_H_INCLUDE

#include <grouplistwidget/grouplistwidget-config.h>
#include <QSortFilterProxyModel>
#include <QList>

QT_BEGIN_NAMESPACE
class QAbstractListModel;
QT_END_NAMESPACE

class GrouSubModel;
class GroupCustomizer;

//! A Qt model that is to be installed into a GroupListWidget.
class GROUPLISTWIDGET_EXPORT GroupModel : public QSortFilterProxyModel  {
    Q_OBJECT

public:

    //! Create a new model from a table or view definition and a database.
    GroupModel (
        QObject * parent = NULL);

    Q_DISABLE_COPY(GroupModel)

    //! Destructor.
    virtual ~GroupModel();

    //! Tell the number of groups.
    int
    groupCount () const {
        return groups_.count ();
    }

    void setSourceModel (
            QAbstractItemModel * source_model) Q_DECL_OVERRIDE;

    //! Retreive index of the column used for grouping.
    inline int
    groupingCol () const {
        return col_grouping_;
    }

    //! Set index of the column used for grouping.
    bool
    setGroupingCol (
            int value);

    //! Retreive index of the column provinding the large image .
    inline int
    largeImageCol () const {
        return col_image_large_;
    }

    //! Set index of the column provinding the large image .
    inline void
    setLargeImageCol (int value) {
        col_image_large_ = value;
    }

    //! Retreive index of the column provinding the small image .
    inline int
    smallImageCol () const {
        return col_image_small_;
    }

    //! Set index of the column provinding the small image .
    inline void
    setSmallImageCol (int value) {
        col_image_small_ = value;
    }


    //! Retreive the object that customizes the behaviour of this class.
    inline GroupCustomizer *
    customizer () const {
        return cust_;
    }

    //! Take ownership of the object that customizes the behaviour of this class.
    inline GroupCustomizer *
    takeCustomizer () {
        GroupCustomizer * result = cust_;
        cust_ = NULL;
        return result;
    }

    //! Set the object that customizes the behaviour of this class.
    inline void
    setCustomizer (
            GroupCustomizer * value);

    //! Label for the group with given index.
    QString
    groupLabel (
            int idx) const;

    //! Label for the group with given index.
    QString
    groupName (
            int idx) const;

    //! The model to be used with a list for a group
    QAbstractListModel *
    groupList (
        int idx) const;

protected:

    void
    buildAllGroups();

    void
    buildNoGroupingGroup();

    void
    clearAllGroups ();

    GrouSubModel *
    subModelByName (
            const QString & s_name) const;

    int
    subIndexByName (
            const QString & s_name) const;


private:

    int col_grouping_; /**< the index of the column used for grouping */
    int col_image_large_; /**< the index of the column provinding the large image */
    int col_image_small_; /**< the index of the column provinding the small image */
    QList<GrouSubModel*> groups_; /**< the list of groups */
    GroupCustomizer * cust_; /**< object used to customize */

}; // class GroupModel

#endif // GUARD_GROUPMODEL_H_INCLUDE
