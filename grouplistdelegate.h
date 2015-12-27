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
#include <QStyledItemDelegate>

class GroupListWidget;
class GroupModel;

//! Allows user to customize parts of the GroupModel.
class GROUPLISTWIDGET_EXPORT GroupListDelegate : public QStyledItemDelegate/* QAbstractItemDelegate */ {
    Q_OBJECT
public:

    enum Layout {
        LayInvalid = -1,

        LayIcon, /**< Icon at the top, text at the bottom */
        LayList, /**< Icon at left side, text at right */

        Laymax
    };

public:

    //! Constructor.
    GroupListDelegate (
            QObject *parent = NULL);

    //! Destructor.
    virtual ~GroupListDelegate () {}

    //! compute cached values
    virtual void
    reinit (
            GroupListWidget * lwidget,
            GroupModel * umodel);

    //! Get the size of the cell.
    QSize
    gridCell () const {
        return item_size_;
    }

protected:

    void
    paint (
            QPainter * painter,
            const QStyleOptionViewItem & option,
            const QModelIndex & index ) const;

    QSize
    sizeHint (
            const QStyleOptionViewItem & option,
            const QModelIndex & index ) const;


private:
    QSize item_size_; /**< cached size of the entire item */
    QRect pix_pos_; /**< the position of the pixmap inside the item rect */
    QRect text_pos_; /**< the position of the first label inside the item rect */
    Layout layout_; /**< the way internal components are arranged */
    int lay_count_; /**< the number of labels to show */

}; // class GroupListDelegate

#endif // GUARD_GROUP_LIST_DELEGATE_H_INCLUDE
