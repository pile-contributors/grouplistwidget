/**
 * @file dbmodel.h
 * @brief Declarations for DbModel class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_GROUPCUSTOMIZER_H_INCLUDE
#define GUARD_GROUPCUSTOMIZER_H_INCLUDE

#include <grouplistwidget/grouplistwidget-config.h>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class GrouModel;

//! Allows user to customize parts of the GroupModel.
class GROUPLISTWIDGET_EXPORT GroupCustomizer {

public:

    //! Constructor.
    GroupCustomizer (GrouModel * target) : m_(target) {}

    //! Destructor.
    virtual ~GroupCustomizer () {}

    //! Retreive the name for a group.
    virtual QString
    name (
            const QModelIndex & index);

    //! Retreive the label for a group.
    virtual QString
    label (
            const QModelIndex & index,
            const QString & name);


private:
    GrouModel * m_;
}; // class GroupCustomizer

#endif // GUARD_GROUPCUSTOMIZER_H_INCLUDE
