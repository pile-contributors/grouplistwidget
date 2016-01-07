/**
 * @file groupm_columns.h
 * @brief Declarations for GroupModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */
#ifndef GUARD_GROUPMCOLUMNS_H_INCLUDE
#define GUARD_GROUPMCOLUMNS_H_INCLUDE

#include <grouplistwidget/groupmodel.h>

//! .
class GROUPLISTWIDGET_EXPORT GroupMColumns : public GroupModel {
    Q_OBJECT
public:

    //! Default constructor.
    GroupMColumns (
            QAbstractItemModel *model = NULL,
            const QList<int> & grp_cols = QList<int>(),
            const QList<int> & sort_cols = QList<int>(),
            QObject * parent = NULL);

    //! Destructor.
    virtual ~GroupMColumns();

    //! Retrieve the list of columns where grouping can be applied.
    virtual QList<int>
    groupingColumns () const {
        return grp_cols_;
    }

    //! Retrieve the list of columns where sorting can be applied.
    virtual QList<int>
    sortingColumns () const {
        return sort_cols_;
    }

    //! List of grouping columns.
    void
    setGroupingColumns (
            const QList<int> & value) {
        grp_cols_ = value;
    }

    //! List of sorting columns.
    void
    setSortingColumns (
            const QList<int> & value) {
        sort_cols_ = value;
    }

private:

    QList<int> grp_cols_; /**< list of grouping columns */
    QList<int> sort_cols_; /**< list of grouping columns */

public: virtual void anchorVtable() const;
}; // class GroupModel

#endif // GUARD_GROUPMCOLUMNS_H_INCLUDE
