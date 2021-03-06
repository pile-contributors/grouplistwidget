/**
 * @file grouplistwidget.h
 * @brief Declarations for GroupListWidget class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2015 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_GROUPLISTWIDGET_H_INCLUDE
#define GUARD_GROUPLISTWIDGET_H_INCLUDE

#include <grouplistwidget/grouplistwidget-config.h>
#include <QTreeWidget>
#include <QListView>
#include <QIcon>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
QT_END_NAMESPACE

class GroupSubModel;
class GroupModel;
class GroupListGroup;
class GrpTreeDeleg;

//! A list widget that can group the items.
class GROUPLISTWIDGET_EXPORT GroupListWidget : public QTreeWidget {
    Q_OBJECT

    friend class GrpTreeDeleg;

public:

    //! Default constructor.
    explicit GroupListWidget (
            QWidget *parent = NULL);

    //! Destructor.
    virtual ~GroupListWidget();

    //! Sets the user model in underlying model and updates the view.
    void
    setBaseModel (
            QAbstractItemModel * model);

    //! Retrieve user model from underlying model.
    QAbstractItemModel *
    baseModel () const;

    //! Sets the underlying model and updates the view.
    void
    setUnderModel (
            GroupModel * model);

    //! Retrieve underlying model.
    GroupModel *
    underModel () const {
        return m_;
    }

    //! Take ownership of underlying model.
    GroupModel *
    takeUnderModel (
            GroupModel * other = NULL);

    //! Adds actions for sorting, grouping to the menu.
    void
    appendToMenu (
            QMenu * menu);

    //! Adds actions for grouping to the menu.
    void
    appendGroupToMenu (
            QMenu * menu,
            bool directly = false);

    //! Adds actions for sorting to the menu.
    void
    appendSortToMenu (
            QMenu * menu,
            bool directly = false);

    //! Adds actions for arranging the items to the menu.
    void
    appendLayoutToMenu (
            QMenu * menu);

    //! Place text to the right ("list") or to the bottom ("icon")
    QListView::ViewMode
    viewMode () const {
        return list_view_mode_; }

    //! Arrange items from left to right or from top to bottom
    QListView::Flow
    flow () const {
        return list_flow_; }

    //! Arrange items from left to right or from top to bottom
    int
    pixmapSize () const {
        return pixmap_size_; }

    //! Arrange items from left to right or from top to bottom
    QSize
    gridCell () {
        if (grid_cell_.isNull()) {
            grid_cell_ = computeGridCell ();
        }
        return grid_cell_;
    }

    //! Arrange items from left to right or from top to bottom
    QSize
    computeGridCell () const;

    //! The delegate used with list items, if any.
    QAbstractItemDelegate *
    listDelegate () const {
        return list_delegate_;
    }

    //! Set the delegate to be used with list items.
    void
    setListDelegate (
            QAbstractItemDelegate * value);

    //! Set the delegate to be used with list items.
    QAbstractItemDelegate *
    takeListDelegate (
            QAbstractItemDelegate * value = NULL);

    //! The index of current item or -1 if none.
    int
    blueItem () const {
        return current_row_;
    }

    //! Change the index of current item (-1 to clear selection).
    void
    setBlueItem (
            int value);

    //! Change the icon shown to the left of text when the group is expanded.
    void
    setIconExpanded (
            const QIcon & value) {
        icon_group_expanded_ = value;
    }

    //! Icon shown to the left of text when the group is expanded.
    const QIcon &
    iconExpanded () {
        return icon_group_expanded_;
    }

    //! Change the icon shown to the left of text when the group is collapsed.
    void
    setIconCollapsed (
            const QIcon & value) {
        icon_group_collapsed_ = value;
    }

    //! Icon shown to the left of text when the group is collapsed.
    const QIcon &
    iconCollapsed () {
        return icon_group_collapsed_;
    }

    //! Change the color for group background.
    void
    setGroupBackColor (
            const QColor & value) {
        group_back_ = value;
    }

    //! The color for group background.
    const QColor &
    groupBackColor () {
        return group_back_;
    }



public slots:

    //! Place text to the right ("list") or to the bottom ("icon")
    void
    setViewMode (
            QListView::ViewMode value);

    //! Arrange items from left to right or from top to bottom
    void
    setFlow (
            QListView::Flow value);

    //! Change the size of the icons.
    void
    setPixmapSize (
            int value);

    //! Change the size of the icons.
    void
    increasePixSize () {
        int new_size;
        if (pixmap_size_ == -1) {
            new_size = 48;
        } else {
            new_size = static_cast<int> (pixmap_size_ * 1.2);
        }
        setPixmapSize (new_size);
    }

    //! Change the size of the icons.
    void
    decreasePixSize () {
        int new_size;
        if (pixmap_size_ == -1) {
            new_size = 48;
        } else {
            new_size = static_cast<int> (pixmap_size_ * 0.8);
            if (new_size < 16) new_size = 16;
        }
        setPixmapSize (new_size);
    }

signals:

    //! Informs that current item has changed.
    void
    currentLVItemChanged (
            int row,
            int previous_row);

    //! Informs that current item has changed.
    void
    currentLVItemChangedEx (
            int row_in_base,
            int row_in_list,
            int gsm);

private slots:

    //! Before the actual changes are implemented.
    void
    underModelAboutToBeReset ();

    //! After the model was updated.
    void
    underModelReset ();

    //! The direction (NOT the column) of the grouping has changed.
    void
    underGroupingChanged (
            int column,
            Qt::SortOrder);

    //! The selection in a listview changes.
    void
    listViewSelChange (
            const QModelIndex &current,
            const QModelIndex &previous);

    //! The actual function and parameters are extracted from sender properties.
    void
    genericSlot ();

    //! An item in treeview has been clicked.
    void
    itemClicked (
            const QModelIndex & item);

protected:

    //! Implement a default contextual menu.
    void
    contextMenuEvent (
            QContextMenuEvent * event);

    //! The widget is being resized.
    void
    resizeEvent (
            QResizeEvent *e);

    //! Change the size of the image using the wheels.
    void
    wheelEvent (
            QWheelEvent *);

private:

    //! Populates the widget based on the current state of the model.
    void
    recreateFromGroup();

    //! Connects required signals.
    void
    installUnderModel (
            GroupModel * value);

    //! Disconnects required signals.
    void
    uninstallUnderModel (
            bool b_delete = false);

    //! Create one internal widget for group content.
    GroupListGroup *
    createListView (
            GroupSubModel *smdl,
            QTreeWidgetItem *tvi);

    //! Helper used to construct the contextual menu.
    QMenu *
    addColumnsToMenu (
            QMenu *menu,
            const QString &menu_label,
            const QList<int> &gcol_lst,
            const QStringList &gcol_lbl,
            int crt_grp,
            const char *connect_to,
            bool directly);

    //! Makes sure that the lists show all their content.
    void
    arangeLists ();

    //! Let the delegate cache geometry.
    void
    reinitDelegate ();

    QSize
    gridCellFromDelegate (
            QAbstractItemDelegate *delegate,
            QStyleOptionViewItem & option) const;


public:

    static QLatin1String nameGroupAscending () {
        return QLatin1String ("GroupAscending");
    }

    static QLatin1String nameGroupDescending () {
        return QLatin1String ("GroupDescending");
    }

    static QLatin1String nameGroupDestroy () {
        return QLatin1String ("GroupDestroy");
    }

    static QLatin1String nameSortAscending () {
        return QLatin1String ("SortAscending");
    }

    static QLatin1String nameSortDescending () {
        return QLatin1String ("SortDescending");
    }

    static QLatin1String nameSortDestroy () {
        return QLatin1String ("SortDestroy");
    }

    static QLatin1String nameLayoutList () {
        return QLatin1String ("LayoutList");
    }

    static QLatin1String nameLayoutIcons () {
        return QLatin1String ("LayoutIcons");
    }

    static QLatin1String nameLayoutZoomIn () {
        return QLatin1String ("LayoutZoomIn");
    }

    static QLatin1String nameLayoutZoomOut () {
        return QLatin1String ("LayoutZoomOut");
    }


    GroupModel * m_; /**< the underlying model */
    bool resize_guard_; /**< is it OK to react to resize? */
    QListView::ViewMode list_view_mode_; /**< can be either list (text on the right) or icon (text beneath) */
    QListView::Flow list_flow_; /**< arrange items from left to right or from top to bottom */
    int pixmap_size_; /**< the size of the image (-1 is unconstrained */
    QAbstractItemDelegate * list_delegate_; /**< the delegate used with lists */
    QSize grid_cell_; /**< the dimension for the grid cell */
    int current_row_; /**< The index of the current row */
    QIcon icon_group_expanded_; /**< Icon shown to the left of text when the group is expanded. */
    QIcon icon_group_collapsed_; /**< Icon shown to the left of text when the group is collapsed. */
    QColor group_back_; /**< the color for group background */
}; // GroupListWidget

#endif // GUARD_GROUPLISTWIDGET_H_INCLUDE
