/**
 * @file grouplistwidget.cc
 * @brief Definitions for GroupListWidget class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "grouplistwidget.h"
#include "groupmodel.h"
#include "groupsubmodel.h"
#include "grouplistgroup.h"
#include "grouplistdelegate.h"

#include "grouplistwidget-private.h"

#include <QTreeWidgetItem>
#include <QListView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QStyledItemDelegate>
#include <QFontMetrics>
#include <QWheelEvent>


#define GEN_SLOT_FUN "gsfunction"
#define GEN_SLOT_ARG "gsargument"
#define GEN_SLOT_FUN_FLOW "gsf_Flow"
#define GEN_SLOT_FUN_MODE "gsf_Mode"

#define GEN_SLOT_ACTION(__f__, __label__, __getter__, __value__) \
    act = mnu->addAction ( \
            __label__, this, SLOT(genericSlot())); \
    act->setCheckable (true); \
    act->setChecked (__getter__ == __value__); \
    act->setProperty (GEN_SLOT_FUN, GEN_SLOT_FUN_ ## __f__); \
    act->setProperty (GEN_SLOT_ARG, __value__);



/* ------------------------------------------------------------------------- */
//! Represents a group inside the widget.
class GrpTreeItem : public QTreeWidgetItem {
public:
    int group_index_;
    GroupListGroup * lv_;
    GroupSubModel * gsm_;

    GrpTreeItem (const QString & label, int idx, GroupSubModel * gsm) :
        QTreeWidgetItem (QStringList () << label ),
        group_index_ (idx),
        lv_(NULL),
        gsm_(gsm)
    {
    }
public: virtual void anchorVtable() const;
};
void GrpTreeItem::anchorVtable() const {}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
//! Delegate for QTreeWidget.
class GrpTreeDeleg : public QStyledItemDelegate {
public:
    QAbstractItemDelegate * prev_;

    GrpTreeDeleg (QAbstractItemDelegate * prev, GroupListWidget *parent = NULL) :
        QStyledItemDelegate (parent),
        prev_(prev)
    {
    }

    GroupListWidget * glw () const {
        return static_cast<GroupListWidget*>(parent());
    }

    //! Return the size of the widget if there is one.
    QSize sizeHint (
            const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        /*for (;;) {

            if (!index.isValid())
                break;
            GroupListWidget * tv = glw();
            QWidget * w = tv->itemWidget(tv->itemFromIndex (index), 0);
            if (w == NULL)
                break;
            return w->size();
        }*/

        if (prev_ != NULL) {
            return prev_->sizeHint (option, index);
        } else {
            return QStyledItemDelegate::sizeHint (option, index);
        }
    }

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (prev_ != NULL) {
            return prev_->paint (painter, option, index);
        } else {
            return QStyledItemDelegate::paint (painter, option, index);
        }
    }

public: virtual void anchorVtable() const;
};
void GrpTreeDeleg::anchorVtable() const {}
/* ========================================================================= */



/**
 * @class GroupListWidget
 *
 * For simplest use case the user must install at least a base model
 * using setBaseModel().
 *
 * The instance will always have an internal model accesible by
 * using underModel(). At construction
 * time and if the user calls takeUnderModel() with no arguments
 * a default GroupModel is created and used. setUnderModel() will
 * refuse to install a NULL model.
 *
 * The instance owns the internal GroupModel that is destroyed in the
 * destructor. To avoid the destructor the user may choose to
 * takeUnderModel() before the destructor kicks in.
 */

/* ------------------------------------------------------------------------- */
/**
 * For the instance to be usable a model needs to be installed.
 */
GroupListWidget::GroupListWidget (QWidget *parent) :
    QTreeWidget (parent),
    m_(new GroupModel()),
    resize_guard_(false),
    list_view_mode_(QListView::IconMode),
    list_flow_(QListView::LeftToRight),
    //list_flow_(QListView::TopToBottom),
    pixmap_size_(-1),
    list_delegate_(NULL),
    grid_cell_()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    setItemDelegate (new GrpTreeDeleg(itemDelegate (), this));
    setHeaderHidden (true);
    setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
    installUnderModel (m_);
    if ((m_ != NULL) && (m_->groupCount() > 0))
        recreateFromGroup ();
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The destructr will also destruct internal model.
 */
GroupListWidget::~GroupListWidget()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    uninstallUnderModel (true);
    if (list_delegate_ != NULL) {
        delete list_delegate_;
        list_delegate_ = NULL;
    }
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This is a convenience function that installs the base model into internal
 * model. It is equivalent to:
 *
 * @code
 * model()->setBaseModel (model);
 * @endcode
 *
 * @param model The model that provides the data to sort, group and present.
 */
void GroupListWidget::setBaseModel (QAbstractItemModel *model)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    m_->setBaseModel (model);
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This is a convenience function that inquires internal model for
 * base model that it uses. It is equivalent to:
 *
 * @code
 * model()->baseModel ();
 * @endcode
 *
 */
QAbstractItemModel *GroupListWidget::baseModel() const
{
    return m_->baseModel ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Please note that - most of the tme - you do not need to derive a class
 * from GroupModel. Instead, install your own model inside the default
 * GroupModel instance and provide a customizer.
 *
 * This instance takes ownership of provided pointer. Currently installed
 * model will be deleted by this method. To avoid this use takeUnderModel()
 * prior to calling this method.
 *
 * @param model The instance to use as internal model.
 */
void GroupListWidget::setUnderModel (GroupModel *model)
{
    GROUPLISTWIDGET_TRACE_ENTRY;

    if (model == NULL) {
        GROUPLISTWIDGET_DEBUGM("Underlying model cannot be NULL\n");
        return;
    }
    if (m_ == model) {
        GROUPLISTWIDGET_DEBUGM("Attempt to install same model "
                               "twice blocked.\n");
        return;
    }
    uninstallUnderModel (true);
    installUnderModel (model);
    recreateFromGroup ();
    update ();

    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 *
 * @param other A replacement to be installed in this instance; if NULL is
 *              provided a default instance will be constructed.
 * @return previously installed model
 */
GroupModel *GroupListWidget::takeUnderModel (GroupModel *other)
{
    GROUPLISTWIDGET_TRACE_ENTRY;

    GroupModel * tmp = m_;
    uninstallUnderModel ();
    if (other == NULL) {
        other = new GroupModel ();
    }
    installUnderModel (other);
    recreateFromGroup ();
    update ();

    GROUPLISTWIDGET_TRACE_EXIT;
    return tmp;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QMenu * GroupListWidget::addColumnsToMenu (
        QMenu *menu, const QString & menu_label,
        const QList<int> & gcol_lst, const QStringList & gcol_lbl,
        int crt_grp, const char * connect_to)
{
    QMenu * mgroup = menu->addMenu (menu_label);

    Q_ASSERT(gcol_lst.count() == gcol_lbl.count());
    int i = 0;
    foreach (const QString & s_col, gcol_lbl) {
        QAction * act = mgroup->addAction (s_col, m_, connect_to);
        act->setCheckable (true);
        act->setChecked (crt_grp == i);
        act->setProperty ("columnIndex", i);

        ++i;
    }

    mgroup->addSeparator();
    return mgroup;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::appendGroupToMenu (QMenu *menu)
{
    int crt_grp = -1;
    QMenu * mgroup = addColumnsToMenu (
                menu, tr ("Group by:"),
                m_->groupingColumns (), m_->groupingColumnLabels (&crt_grp),
                crt_grp, SLOT(setGroupingColumnByProperty()));

    QAction * act_gr_asc = mgroup->addAction (
                tr ("Ascending"), m_, SLOT(setGroupingAscending()));
    act_gr_asc->setCheckable (true);
    act_gr_asc->setChecked (m_->groupingDirection() == Qt::AscendingOrder);

    QAction * act_gr_desc = mgroup->addAction (
                tr ("Descending"), m_, SLOT(setGroupingDescending()));
    act_gr_desc->setCheckable (true);
    act_gr_desc->setChecked (m_->groupingDirection() == Qt::DescendingOrder);

    mgroup->addSeparator();
    QAction * act_ungroup = mgroup->addAction (
                tr ("Ungrouped"), m_, SLOT(removeGrouping()));
    act_ungroup->setEnabled (m_->isGrouping ());
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::appendSortToMenu (QMenu *menu)
{
    int crt_sort = -1;
    QMenu * msort = addColumnsToMenu (
                menu, tr ("Sort by:"),
                m_->sortingColumns (), m_->sortingColumnLabels (&crt_sort),
                crt_sort, SLOT(setSortingColumnByProperty()));

    QAction * act_srt_asc = msort->addAction (
                tr ("Ascending"), m_, SLOT(setSortingAscending()));
    act_srt_asc->setCheckable (true);
    act_srt_asc->setChecked (m_->sortingDirection() == Qt::AscendingOrder);

    QAction * act_srt_desc = msort->addAction (
                tr ("Descending"), m_, SLOT(setSortingDescending()));
    act_srt_desc->setCheckable (true);
    act_srt_desc->setChecked (m_->sortingDirection() == Qt::DescendingOrder);

    msort->addSeparator();
    QAction * act_unsort = msort->addAction (
                tr ("Unsorted"), m_, SLOT(removeSorting()));
    act_unsort->setEnabled (m_->isSorting ());
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::appendLayoutToMenu (QMenu *menu)
{

    QMenu * mnu = menu->addMenu (tr ("Layout:"));
    QAction * act;

    GEN_SLOT_ACTION(MODE, tr ("List View"), viewMode(), QListView::ListMode);
    GEN_SLOT_ACTION(MODE, tr ("Icon View"), viewMode(), QListView::IconMode);
    mnu->addSeparator();
    /*
    GEN_SLOT_ACTION(FLOW, tr ("Left to Right"), flow (), QListView::LeftToRight);
    GEN_SLOT_ACTION(FLOW, tr ("Top to Bottom"), flow (), QListView::TopToBottom);
    mnu->addSeparator();*/
    act = mnu->addAction (tr ("Zoom in"), this, SLOT(increasePixSize()));
    act->setEnabled (m_->pixmapColumn() != -1);
    act = mnu->addAction (tr ("Zoom out"), this, SLOT(decreasePixSize()));
    act->setEnabled (m_->pixmapColumn() != -1);

}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The widget takes ownership of the provided pointer and it will destroy it
 * in its own destructor. Use takeListDelegate to avoid this behaviour.
 *
 * @param value new delegate to use or NULL to revert to using default delegate.
 */
void GroupListWidget::setListDelegate (QAbstractItemDelegate *value)
{
    if (list_delegate_ == value)
        return;
    if (list_delegate_ != NULL) {
        delete list_delegate_;
    }
    list_delegate_ = value;

    reinitDelegate ();

    if (m_->baseModel() != NULL) {
        // update lists currently visible
        int i_max = topLevelItemCount();
        for (int i = 0; i < i_max; ++i) {
            GrpTreeItem * it = static_cast<GrpTreeItem *>(topLevelItem (i));
            it->lv_->setItemDelegate (value);
        }
        arangeLists ();
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The widget takes ownership of the provided pointer and it will destroy it
 * in its own destructor. Use takeListDelegate to avoid this behaviour.
 *
 * @param value new delegate to use or NULL to revert to using default delegate.
 */
QAbstractItemDelegate * GroupListWidget::takeListDelegate (QAbstractItemDelegate *value)
{
    if (list_delegate_ == value)
        return NULL;
    QAbstractItemDelegate * tmp = list_delegate_;
    list_delegate_ = NULL;
    setListDelegate (value);
    return tmp;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::setViewMode (QListView::ViewMode value)
{
    if (list_view_mode_ == value)
        return;
    list_view_mode_ = value;
    reinitDelegate ();
    if (m_->baseModel() != NULL) {
        // update lists currently visible
        int i_max = topLevelItemCount();
        for (int i = 0; i < i_max; ++i) {
            GrpTreeItem * it = static_cast<GrpTreeItem *>(topLevelItem (i));
            it->lv_->setViewMode (value);
            it->lv_->setWrapping (true);
        }
        arangeLists ();
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::setFlow (QListView::Flow value)
{
    if (list_flow_ == value)
        return;
    list_flow_ = value;
    reinitDelegate ();
    if (m_->baseModel() != NULL) {
        // update lists currently visible
        int i_max = topLevelItemCount();
        for (int i = 0; i < i_max; ++i) {
            GrpTreeItem * it = static_cast<GrpTreeItem *>(topLevelItem (i));
            it->lv_->setFlow (value);
            it->lv_->setWrapping (true);
        }
        arangeLists ();
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QSize GroupListWidget::gridCellFromDelegate (
        QAbstractItemDelegate * delegate, QStyleOptionViewItem & option) const
{
    GroupListDelegate * our_del =
            qobject_cast<GroupListDelegate*>(list_delegate_);
    if (our_del != NULL) {
        return our_del->gridCell();
    } else {
        QModelIndex midx;
        return delegate->sizeHint (option, midx);
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QSize GroupListWidget::computeGridCell () const
{
    QSize sz;
    int i_max = topLevelItemCount();
    if ((m_->baseModel() != NULL) && (i_max > 0)) {
        // update lists currently visible
        GrpTreeItem * it = static_cast<GrpTreeItem *>(topLevelItem (0));
        if (list_delegate_ != NULL) {
            sz = gridCellFromDelegate (list_delegate_, it->lv_->viewOptions ());
        } else {
            QAbstractItemDelegate * d = it->lv_->itemDelegate();
            if (d != NULL) {
                sz = gridCellFromDelegate (d, it->lv_->viewOptions ());
                QSize autocmop;
                if (list_view_mode_ == QListView::ListMode) {
                    autocmop = QSize (2 + pixmap_size_ + 2 + pixmap_size_ + 2, 2 + pixmap_size_ + 2);
                } else {
                    QFontMetrics fm(font());

                    autocmop = QSize (2 + pixmap_size_ + 2, 2 + pixmap_size_ + 2 + fm.height()*1.2 + 2);
                }
                sz = QSize (qMax (sz.width(), autocmop.width()),
                            qMax (sz.height(), autocmop.height()));
            } else {
                if (list_view_mode_ == QListView::ListMode) {
                    sz = QSize (2 + pixmap_size_ + 2 + pixmap_size_ + 2, 2 + pixmap_size_ + 2);
                } else {
                    QFontMetrics fm(font());
                    sz = QSize (2 + pixmap_size_ + 2, 2 + pixmap_size_ + 2 + fm.height()*1.2 + 2);
                }
            }
        }
    }
    return sz;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::setPixmapSize (int value)
{
    if (pixmap_size_ == value)
        return;
    pixmap_size_ = value;
    grid_cell_ = computeGridCell ();


    reinitDelegate ();
    arangeLists ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::reinitDelegate ()
{
    // if the delegate is our own kind of delegate, ask it to compute
    // and cache display values
    GroupListDelegate * our_del =
            qobject_cast<GroupListDelegate*>(list_delegate_);
    if (our_del != NULL) {
        our_del->reinit (this, underModel());
        int i_max = topLevelItemCount();
        QSize sz = our_del->gridCell ();
        for (int i = 0; i < i_max; ++i) {
            GrpTreeItem * it = static_cast<GrpTreeItem *>(topLevelItem (i));
            //GroupSubModel * subm = static_cast<GroupSubModel *>(it->lv_->model());
            //subm->signalReset ();
            it->lv_->setGridSize (sz);
        }
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The method simply uses more specialized methods to insert actions for
 * grouping, sorting and laying out the items (in this order).
 *
 * All actions are constructed on the fly and are parented in their respective
 * menu. As a result, when \b menu gets destroyed the same happens to all
 * actions and menus created by this method.
 *
 * @param menu The menu where new components will be appended.
 */
void GroupListWidget::appendToMenu (QMenu *menu)
{
    appendGroupToMenu (menu);
    appendSortToMenu (menu);
    appendLayoutToMenu (menu);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::underModelAboutToBeReset()
{
    clear ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::underModelReset()
{
    reinitDelegate ();
    recreateFromGroup ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::underGroupingChanged (int, Qt::SortOrder)
{
    recreateFromGroup ();
#if 0
    if (!m_->isGrouping())
        return;
    QList<GrpTreeItem*> lst_;
    lst_.reserve (topLevelItemCount());
    while (topLevelItemCount() > 0) {
        GrpTreeItem * it = static_cast<GrpTreeItem *>(topLevelItem (0));
        takeTopLevelItem (0);
        lst_.prepend (it);
        it->lv_ = NULL;
    }
    int i_max = lst_.count();
    foreach(GrpTreeItem * iter, lst_) {
        addTopLevelItem (iter);
        iter->lv_ = createListView (iter->gsm_, iter->child (0));
        iter->group_index_ = i_max - iter->group_index_ - 1;
        iter->setExpanded (true);
    }
    arangeLists ();
#endif
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::listViewSelChange (
        const QModelIndex &current, const QModelIndex &)
{
    if (!current.isValid())
        return;
    const GroupSubModel * gsm =
            static_cast<const GroupSubModel *>(current.model());
    if (gsm == NULL) {
        Q_ASSERT(false);
        return;
    }

    // make sure that only a single listview item is selected across all
    // lists
    int i_max = topLevelItemCount();
    for (int i = 0; i < i_max; ++i) {
        GrpTreeItem * iter = static_cast<GrpTreeItem *>(topLevelItem(i));
        if (iter->gsm_ == gsm) {
            // found the model that trigered the event
        } else if (iter->lv_ != NULL){
            iter->lv_->setCurrentIndex (QModelIndex());
        }
    }
    int row_in_list = current.row();
    int row_in_base = gsm->mapRowToBaseModel (row_in_list);
    emit currentLVItemChanged (row_in_base);
    emit currentLVItemChangedEx (row_in_base, row_in_list, gsm->listIndex ());
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::genericSlot ()
{
    QObject * s = sender ();
    QString s_func = s->property (GEN_SLOT_FUN).toString ();
    if (s_func == QLatin1String (GEN_SLOT_FUN_FLOW)) {
        setFlow ((QListView::Flow)s->property (GEN_SLOT_ARG).toInt ());
    } else if (s_func == QLatin1String (GEN_SLOT_FUN_MODE)) {
        setViewMode ((QListView::ViewMode)s->property (GEN_SLOT_ARG).toInt ());
    } else {
        Q_ASSERT(false);
    }
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::contextMenuEvent (QContextMenuEvent *event)
{
    QPoint evpos = event->globalPos();
    if (event->reason() != QContextMenuEvent::Mouse) {
        evpos = mapToGlobal (QPoint(pos().x() + size().width() / 2,
                                    pos().y() + size().height() / 2));
    }

    QMenu ctx;
    appendToMenu (&ctx);
    ctx.exec (evpos);

    event->accept();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::resizeEvent (QResizeEvent *e)
{
    if (resize_guard_)
        return;
    resize_guard_ = true;

    arangeLists ();

    e->accept();
    resize_guard_ = false;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::wheelEvent (QWheelEvent * event)
{
    if ((event->modifiers () & Qt::ControlModifier) == Qt::ControlModifier) {
        int d = event->delta() / 120;
        while (d > 0) {
            increasePixSize();
            --d;
        }
        while (d < 0) {
            decreasePixSize();
            ++d;
        }
        event->accept();
        return;
    }
    QTreeWidget::wheelEvent (event);
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::arangeLists ()
{
    int i_max = topLevelItemCount();
    for (int i = 0; i < i_max; ++i) {
        // get the visual rectangle of the last item
        GrpTreeItem * it = static_cast<GrpTreeItem *>(topLevelItem (i));
        GroupListGroup * lv = it->lv_;
        GroupSubModel * gsm = it->gsm_;
        if ((lv != NULL) && (gsm != NULL)) {
            for (int j = 0; j < 2; ++j) {
                lv->doItemsLayout();
                QRect r = lv->visualRect (
                            gsm->index (gsm->rowCount() - 1, 0));
                int addf = lv->frameWidth() * 2 + 4;
                int this_width = size().width();
                int new_width = this_width - lv->pos().x();
                if (new_width < r.width() + 2)
                    new_width = r.width() + 2;
                QSize new_size (new_width, r.bottom() + addf);

                lv->setMinimumSize (new_size);
                lv->setMaximumSize (new_size);
                lv->resize (new_size);
                lv->setSizePolicy (QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
                if (it->childCount() > 0) {
                    it->child (0)->setSizeHint (0, new_size);
                } else {
                    it->setSizeHint (0, new_size);
                }
            }
        }
    }
    scheduleDelayedItemsLayout ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
GroupListGroup * GroupListWidget::createListView (
        GroupSubModel * smdl, QTreeWidgetItem * tvi)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    GroupListGroup * lv = new GroupListGroup (this);
    if (smdl != NULL) {
        lv->setModel (smdl);
        connect(lv->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &GroupListWidget::listViewSelChange);
    } else {
        Q_ASSERT(false);
    }

    if (tvi != NULL) {
        setItemWidget (tvi, 0, lv);/*
        int vph = lv->viewport()->size().height();
        int addf = lv->frameWidth() * 2;
        lv->setMinimumHeight (vph + addf);
        lv->setMaximumHeight (vph + addf);*/
    }
    GROUPLISTWIDGET_TRACE_EXIT;
    return lv;
}
/* ------------------------------------------------------------------------- */

/* ========================================================================= */
void GroupListWidget::recreateFromGroup ()
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    clear ();
    int i_max = m_->groupCount ();

    if (m_->isGrouping()) {
        for (int i = 0; i < i_max; ++i) {
            GroupSubModel * gsm = m_->group (i);
            GrpTreeItem * tvi = new GrpTreeItem (gsm->label(), i, gsm);
            addTopLevelItem (tvi);
            QTreeWidgetItem * subtvi = new QTreeWidgetItem (tvi);
            tvi->lv_ = createListView (gsm, subtvi);
            tvi->setExpanded (true);
        }
    } else {
        GroupSubModel * gsm = NULL;
        if (i_max == 1) {
            gsm = m_->group (0);
        } else {
            Q_ASSERT(false);
        }
        GrpTreeItem * tvi = new GrpTreeItem (QString("test"), 0, gsm);
        addTopLevelItem (tvi);
        tvi->lv_ = createListView (gsm, tvi);
        tvi->setExpanded (true);
    }
    arangeLists ();
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::installUnderModel (GroupModel *value)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    if (value != NULL) {
        connect (value, &GroupModel::modelAboutToBeReset,
                 this, &GroupListWidget::underModelAboutToBeReset);
        connect (value, &GroupModel::modelReset,
                 this, &GroupListWidget::underModelReset);
        connect (value, &GroupModel::groupingChanged,
                 this, &GroupListWidget::underGroupingChanged);
    }
    reinitDelegate ();
    m_ = value;
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void GroupListWidget::uninstallUnderModel (bool b_delete)
{
    GROUPLISTWIDGET_TRACE_ENTRY;
    if (m_ != NULL) {

        disconnect (m_, &GroupModel::modelAboutToBeReset,
                    this, &GroupListWidget::underModelAboutToBeReset);
        disconnect (m_, &GroupModel::modelReset,
                    this, &GroupListWidget::underModelReset);
        disconnect (m_, &GroupModel::groupingChanged,
                    this, &GroupListWidget::underGroupingChanged);

        if (b_delete)
            delete m_;

        m_ = NULL;
    }
    GROUPLISTWIDGET_TRACE_EXIT;
}
/* ========================================================================= */


