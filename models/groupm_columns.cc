/**
 * @file groupm_columns.h
 * @brief Definitions for GroupModel class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "groupm_columns.h"


/**
 * @class GroupMColumns
 *
 */

GroupMColumns::GroupMColumns (
        QAbstractItemModel *model, const QList<int> &grp_cols,
        const QList<int> &sort_cols, QObject *parent) :
    GroupModel(model, parent),
    grp_cols_(grp_cols),
    sort_cols_(sort_cols)
{
}

GroupMColumns::~GroupMColumns()
{
}

void GroupMColumns::anchorVtable () const {}
