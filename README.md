GroupListWidget
===============

The pile attempts to provide a widget that is capable of presenting
2D data in a list of potentially grouped entries.

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

This simp
