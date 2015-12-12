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


#include <assert.h>

/**
 * @class GroupCustomizer
 *
 * .
 */

/* ------------------------------------------------------------------------- */
/**
 * By default the method simply returns the string found by using
 * Qt::DisplayRole in indicated cell.
 *
 * \param index The position inside GroupModel of the cell that triggers the
 *              creation of this group.
 */
QString GroupCustomizer::name (
        const QModelIndex &index)
{
    return m_->data (index).toString ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * By default the method simply returns the name of the group.
 *
 * \param index The position inside GroupModel of the cell that triggers the
 *              creation of this group.
 * \param name The name as determined by a previous call to name().
 */
QString GroupCustomizer::label (const QModelIndex & index, const QString &name)
{
    return name;
}
/* ========================================================================= */
