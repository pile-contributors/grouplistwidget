GroupListWidget
===============

The pile attempts to provide a widget that is capable of presenting
2D data in a list of (potentially grouped) entries.

The user is expected to install a base model that provides all the data.
Internally, a number of models will be dynamically derived to serve the data
to constituent widgets.

The simplest use case is when the user simply creates the widget
and sets the base model:

    // create a model
    QStandardItemModel * model = new QStandardItemModel(4, 4);
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
            model->setItem(row, column, item);
        }
    }

    // the widget to use that model
    GroupListWidget * widget = new GroupListWidget ();
    widget->setBaseModel (model);
    widget->show ();

This already provides the user with the ability to sort items
and group them. A default contextual menu is installed that allows
these operations along with others and the user is assisted in
building of customized contextual menus via `appendToMenu()`.

GroupModel
----------

The widget uses a `GroupModel` instance to manage the data.
The user can create derived classes from `GroupModel` class to
gain full control over the internal workings of the model.

    // create an instance of a derived model
    GroupDerivedModel * internal_model = new GroupDerivedModel();
    internal_model->setBaseModel (model);

    // the widget to use that model
    GroupListWidget * widget = new GroupListWidget ();
    widget->setUnderModel (internal_model);
    widget->show ();

The icons for the items are retrieved by querying a designated
column for a specific row (`DecorationRole` by default).
The base model is expected to respond
with a `QPixmap`. One can use `setPixmapColumn()` and `setPixmapRole ()`
to customize this behavior. To get rid of all icons call
`setPixmapColumn(-1)`. To get rid of some of the icons return a null
pixmap from user model.

To enable grouping call `setGroupingColumn()` with the index of the
column to be used for grouping (default role is `EditRole` and
it may be changed using `setGroupingRole()`).
This will get rid of old groups and
create new ones from scratch. The process of doing this involves
retrieving the value for that particular column from all records
and comparing that with the values seen so far. The function that
compares values may be provided by the user using `setGroupingFunc()`.
Grouping, just like sorting, can be ascending or descending as set by
`setGroupingDirection()` and can be disabled by calling
`setGroupingColumn(-1)` (this is the default).
All these values may be retrieved using corespondent getters.

To enable sorting of items inside a group call `setSortingColumn()`
and customize the role by using `setSortingRole()` (default is
`EditRole`). The function that compares values may be provided
by the user using `setSortingFunc()`.
Sorting can be ascending or descending as set by
`setSortingDirection()` and can be disabled by calling
`setSortingColumn(-1)` (this is the default). Each of these
setter methods have associated getter methods.

Qt Signals are used to communicate changes in the state of the
model to the views. `GroupModel` uses `modelAboutToBeReset()`,
`modelAboutToBeReset()` and `modelReset()` to communicate with
`GroupListWidget` and `GroupSubModel` uses `modelAboutToBeReset()`
and `modelAboutToBeReset()` to communicate with embedded `QListView`.

GroupSubModel
-------------

The lower level model used to provide data to embedded QListView
widgets is described by the `GroupSubModel` class. `GroupModel`
creates and owns instances of this class and it is mostly of internal use.

GroupListDelegate
-----------------

Some functionality requires the use of an advanced delegate for items that
are part of the list.
