/**
 * @file dbmodel.h
 * @brief Declarations for DbModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_GROUP_LIST_DELEGATE_H_INCLUDE
#define GUARD_GROUP_LIST_DELEGATE_H_INCLUDE

#include <grouplistwidget/grouplistwidget-config.h>
#include <QObject>
#include <QAbstractItemDelegate>

//! Allows user to customize parts of the GroupModel.
class GROUPLISTWIDGET_EXPORT GroupListDelegate : public QAbstractItemDelegate {

public:

    //! Constructor.
    GroupListDelegate (
            QObject *parent = NULL);

    //! Destructor.
    virtual ~GroupListDelegate () {}

protected:
#if 0
    void
    paint (
            QPainter * painter,
            const QStyleOptionViewItem & option,
            const QModelIndex & index ) const;

    QSize
    sizeHint (
            const QStyleOptionViewItem & option,
            const QModelIndex & index ) const;
#endif

private:


}; // class GroupListDelegate

#endif // GUARD_GROUP_LIST_DELEGATE_H_INCLUDE
