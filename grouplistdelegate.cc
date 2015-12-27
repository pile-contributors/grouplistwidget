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

#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QModelIndex>
#include <QRect>
#include <QSize>

/**
 * @class GroupListDelegate
 *
 * .
 */

#define GEN_BORDER 2
#define DECO_TEXT_BORDER 4


/* ------------------------------------------------------------------------- */
GroupListDelegate::GroupListDelegate (QObject * parent) :
    QStyledItemDelegate(parent),
    item_size_(16, 16),
    pix_pos_(),
    text_pos_(),
    layout_(LayInvalid),
    lay_count_(0)
{
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListDelegate::reinit (
        GroupListWidget *lwidget, GroupModel *umodel)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    bool b_ret = false;
    for (;;) {

        // For now the mapping is 1:1, but - in the future, we could add more
        // layouts without changing the implementation for GroupListWidget.
        if (lwidget->viewMode () == QListView::ListMode) {
            layout_ = LayList;
        } else if (lwidget->viewMode () == QListView::IconMode) {
            layout_ = LayIcon;
        } else {
            layout_ = LayInvalid;
            break;
        }

        // retreive additional information from data holders
        int pix_size = lwidget->pixmapSize();
        pix_pos_ = QRect(0, 0, pix_size, pix_size);

        QFontMetrics fm (lwidget->font());
        text_pos_ = QRect(
                    0,
                    0,
                    fm.averageCharWidth() * 32,
                    fm.height() * 1.2 + 1);

        lay_count_ = umodel->labelCount();

        // total height of the labels
        int tot_label_h = lay_count_ * text_pos_.height();

        // compute elements based on the layout
        switch (layout_) {
        case LayList: {
            // the icon is placed to the left side, the text to the right
            // vertically, both are placed in the middle

            item_size_ = QSize (
                        GEN_BORDER + pix_pos_.width() + DECO_TEXT_BORDER + text_pos_.width() + GEN_BORDER,
                        GEN_BORDER + qMax (pix_pos_.height(), tot_label_h) + GEN_BORDER);

            pix_pos_.translate (
                        GEN_BORDER,
                        (item_size_.height() - pix_pos_.height()) / 2);
            text_pos_.translate (
                        GEN_BORDER + pix_pos_.width() + DECO_TEXT_BORDER,
                        (item_size_.height() - tot_label_h) / 2);

            break; }
        case LayIcon: {
            // the icon is placed to the top side, the text to the bottom
            // horizontally, both are placed in the middle

            item_size_ = QSize (
                        GEN_BORDER + qMax (pix_pos_.width(), text_pos_.width()) + GEN_BORDER,
                        GEN_BORDER + pix_pos_.height() + DECO_TEXT_BORDER + tot_label_h + GEN_BORDER);

            pix_pos_.translate (
                        (item_size_.width() - pix_pos_.width()) / 2,
                        GEN_BORDER);

            text_pos_.translate (
                        (item_size_.width() - text_pos_.width()) / 2,
                        GEN_BORDER + pix_pos_.height() + DECO_TEXT_BORDER);

            break; }
        default:
            item_size_ = QSize (16, 16);
        }

        b_ret = true;
        break;
    }

    GROUPLISTWIDGET_DEBUGM("Reinitialized geometry to %d, %d\n",
                           item_size_.width (), item_size_.height ());
    GROUPLISTWIDGET_DEBUGM("- image pos (%d, %d), size (%d, %d)\n",
                           pix_pos_.x (), pix_pos_.y(),
                           pix_pos_.width (), pix_pos_.height ());
    GROUPLISTWIDGET_DEBUGM("- text pos (%d, %d), size (%d, %d)\n",
                           text_pos_.x (), text_pos_.y(),
                           text_pos_.width (), text_pos_.height ());
    GROUPLISTWIDGET_DEBUGM("- layout: %d, labels: %d\n",
                           layout_, lay_count_);

    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListDelegate::paint (
        QPainter * painter, const QStyleOptionViewItem & option,
        const QModelIndex & index ) const
{
    bool selected =
            (option.state & QStyle::State_Selected) == QStyle::State_Selected;

    // if (option.showDecorationSelected) {
    if (selected) {
        painter->setBrush (option.palette.brush (QPalette::Highlight));
    } else {
        painter->setBrush (option.backgroundBrush);
    }

    painter->setFont (option.font);
#ifdef GROUPLISTWIDGET_DEBUG
    painter->setPen (Qt::SolidLine);
#else
    painter->setPen (Qt::NoPen);
#endif
    painter->drawRect (option.rect);
    GROUPLISTWIDGET_DEBUGM("Drawing at pos (%d, %d), size (%d, %d)\n",
                           option.rect.x (), option.rect.y(),
                           option.rect.width (), option.rect.height ());

    QRect pix_rect = pix_pos_.translated (option.rect.topLeft());
    QRect drect;
    QVariant vdeco = index.data(Qt::DecorationRole);
    if ((vdeco.type() == QVariant::Pixmap) || (vdeco.type() == QVariant::Bitmap)) {
        QPixmap icon = qvariant_cast<QPixmap>(vdeco);
        float scale = qMin(
                    (float)pix_rect.width()  / (float)icon.width(),
                    (float)pix_rect.height() / (float)icon.height());
        int dst_width = icon.width() * scale;
        int dst_height = icon.height() * scale;
        drect = QRect(
                    pix_rect.x() + (pix_rect.width()  - dst_width)  / 2,
                    pix_rect.y() + (pix_rect.height() - dst_height) / 2,
                    dst_width, dst_height);
        painter->drawPixmap (drect, icon);
    } else if (vdeco.type() == QVariant::Image) {
        QImage icon = qvariant_cast<QImage>(vdeco);
        float scale = qMin(
                    (float)pix_rect.width()  / (float)icon.width(),
                    (float)pix_rect.height() / (float)icon.height());
        int dst_width = icon.width() * scale;
        int dst_height = icon.height() * scale;
        drect = QRect(
                    pix_rect.x() + (pix_rect.width()  - dst_width)  / 2,
                    pix_rect.y() + (pix_rect.height() - dst_height) / 2,
                    dst_width, dst_height);

        painter->drawImage (drect, icon);
    } else if (vdeco.type() == QVariant::Icon) {
        QIcon icon = qvariant_cast<QIcon>(vdeco);
        icon.paint (painter, pix_rect, Qt::AlignHCenter | Qt::AlignVCenter);
    }
    //if (!option.icon.isNull()) {
    //    option.icon.paint (painter, option.rect, option.decorationAlignment);
    //}
    GROUPLISTWIDGET_DEBUGM("- image pos (%d, %d), size (%d, %d)\n",
                           drect.x (), drect.y(),
                           drect.width (), drect.height ());

    if (selected) {
        painter->setPen (option.palette.color (QPalette::HighlightedText));
    } else {
        painter->setPen (option.palette.color (QPalette::Text));
    }
    // painter->drawText (option.rect, option.displayAlignment, option.text);

    QRect text_rect = text_pos_.translated (option.rect.topLeft());
    GROUPLISTWIDGET_DEBUGM("- first text pos (%d, %d), size (%d, %d)\n",
                           text_rect.x (), text_rect.y(),
                           text_rect.width (), text_rect.height ());

    int i_max = lay_count_;
    for (int i = 0; i < i_max; ++i) {
        if (i == 0) {
            QFont fp = painter->font();
            fp.setBold (true);
            painter->setFont (fp);
        } else if (i == 1) {
            QFont fp = painter->font();
            fp.setBold (false);
            painter->setFont (fp);
        }

        QString s_label = index.data (GroupModel::BaseColRole + i).toString();
        painter->drawText (
                    text_rect,
                    Qt::AlignTop|Qt::AlignLeft,
                    s_label);

        text_rect.translate (0, text_rect.height());
    }

}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QSize GroupListDelegate::sizeHint (
        const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    return item_size_;
}
/* ========================================================================= */
