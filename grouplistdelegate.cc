/**
 * @file grouplistwidget.cc
 * @brief Definitions for GROUPLISTWIDGET class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */
#include "grouplistdelegate.h"
#include "groupmodel.h"
#include "groupsubmodel.h"
#include "grouplistwidget.h"
#include "grouplistwidget-private.h"
#include "grouplistgroup.h"

/**
 * @class GroupListDelegate
 *
 * .
 */

#define GEN_BORDER 2
#define DECO_TEXT_BORDER 2

struct GroupListDelegateGrips {
    const GroupSubModel * smdl;
    GroupModel * mdl;
    const GroupListGroup * gg;
    GroupListWidget * glw;

    int label_h; /**< the height reserved for an individual label */
    int labels_total_h; /**< the height of the labels */
    int pix_sz; /**< the size of the place for image */
    QSize full_size_; /**< the size for the whole item */
};

/* ------------------------------------------------------------------------- */
GroupListDelegate::GroupListDelegate (QObject * parent) :
    QAbstractItemDelegate(parent)
{

}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListDelegate::paint (
        QPainter * painter, const QStyleOptionViewItem & option,
        const QModelIndex & index ) const
{
    GroupListDelegateGrips grips;
    if (!getTheGrips (option.index, option.widget, &grips)) {
        QAbstractItemDelegate::paint (painter, option, index);
        return;
    }

    if (!option.icon.isNull()) {
        switch (option.decorationPosition) {
        case QStyleOptionViewItem::Right:
        case QStyleOptionViewItem::Left: {
            // image is placed on same level as the label
            return QSize (GEN_BORDER + pix_sz + DECO_TEXT_BORDER + qMax(pix_sz, 64) + GEN_BORDER,
                          qMax(GEN_BORDER + pix_sz + GEN_BORDER, lbl_h));
        break; }
        case QStyleOptionViewItem::Bottom:
        case QStyleOptionViewItem::Top: {
            // image is placed on top or below the label
            return QSize (qMax (GEN_BORDER + pix_sz + GEN_BORDER, 64),
                          GEN_BORDER + pix_sz + DECO_TEXT_BORDER + lbl_h + GEN_BORDER);
        break; }
        }
    }


}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool GroupListDelegate::getTheGrips (
        const QModelIndex & index, const QWidget * widget,
        GroupListDelegateGrips * grips) const
{
    bool b_ret = false;
    for (;;) {
        const QAbstractItemModel * m = index.model();
        if (m == NULL) {
            break;
        }
        grips->smdl = qobject_cast<const GroupSubModel *>(m);
        if (grips->smdl == NULL) {
            break;
        }
        grips->mdl = grips->smdl->parentModel ();
        if (grips->mdl == NULL) {
            break;
        }
        grips->gg = qobject_cast<const GroupListGroup *>(widget);
        if (grips->gg == NULL) {
            break;
        }
        grips->glw = grips->gg->parent_;

        b_ret = true;
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
bool GroupListDelegate::getTheGripsAndMetrics (
        const QModelIndex & index, const QWidget * widget,
        GroupListDelegateGrips * grips) const
{
    bool b_ret = false;
    for (;;) {
        if (!getTheGrips (index, widget, grips)) {
            break;
        }

        // compute the height of all labels
        grips->label_h = option.fontMetrics.height() * 1.2;
        grips->labels_total_h = GEN_BORDER + grips.mdl->labelCount() * grips->label_h + GEN_BORDER;

        // get the size of the pixmap
        grips->pix_sz = grips->glw->pixmapSize() ;

        switch (option.decorationPosition) {
        case QStyleOptionViewItem::Right:
        case QStyleOptionViewItem::Left: {
            // image is placed on same level as the label
            grips->full_size_ = QSize (
                        GEN_BORDER + grips->pix_sz + DECO_TEXT_BORDER + qMax(grips->pix_sz, 64) + GEN_BORDER,
                        qMax(GEN_BORDER + grips->pix_sz + GEN_BORDER, grips->label_h));
        break; }
        case QStyleOptionViewItem::Bottom:
        case QStyleOptionViewItem::Top: {
            // image is placed on top or below the label
            grips->full_size_ = QSize (
                        qMax (GEN_BORDER + grips->pix_sz + GEN_BORDER, 64),
                        GEN_BORDER + grips->pix_sz + DECO_TEXT_BORDER + grips->label_h + GEN_BORDER);
        break; }
        }



        b_ret = true;
        break;
    }
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QSize GroupListDelegate::sizeHint (
        const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    GroupListDelegateGrips grips;
    if (!getTheGripsAndMetrics (option.index, option.widget, &grips)) {
        return QSize();
    }

    return grips.full_size_;
    /*
    if (display_size_ == 0) {
        GroupListDelegate * gld = (GroupListDelegate *)this;
        gld->text_line_size_ = option.fontMetrics.height() * 1.2;
        gld->display_size_ =
                text_line_size_ * m_->additionalLines ();
        // option.fontMetrics.xHeight() * 2 * m_->additionalLines ();
    }
    int icsz = m_->iconSize ();
    */
    // return QSize (64, 64);
}
/* ========================================================================= */
