/**
 * @file grouplistwidget.h
 * @brief Declarations for GroupListWidget class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_GROUPLISTWIDGET_H_INCLUDE
#define GUARD_GROUPLISTWIDGET_H_INCLUDE

#include <grouplistwidget/grouplistwidget-config.h>
#include <QTreeWidget>

class GroupModel;

//! A listview that can group the items.
class GROUPLISTWIDGET_EXPORT GroupListWidget : public QTreeWidget {

public:

    //! Default constructor.
    GroupListWidget (
            QWidget *parent = NULL);

    //! Destructor.
    virtual ~GroupListWidget();

    //! Assign a model.
    void
    setGroupModel (
            GroupModel * value);


protected:

private:
    GroupModel * m_;
};

#endif // GUARD_GROUPLISTWIDGET_H_INCLUDE
