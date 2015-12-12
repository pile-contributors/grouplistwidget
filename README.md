GroupListWidget
===============

This pile allows the user to present the data provided by
a model in a list.

The widget interperts only first level (does not use the
parent-child relations). Items are considered to reside on
rows with the columns providing various information.

One of the columns may be set to provide the icon
for the item. Some other column may be used to
arrange itema into groups, with yet another column
being  used for sorting the items within a group.

The items may be presented in standard details, small icons,
large icons format.

Implementation details
----------------------

A QSortFilterProxyModel is used to sort the original
model by desired column and to filter the items.

The model is iterated by the column designated for
grouping and the labels may be extracted from that
column or they may be provided by a callback.

A treewidget is used as the top level widget. Top level
items are the names for the groups wile immediate
children are listviews that each expose one
internal model is created for each group.

The internal model simply keeps a map between its own rows
and the rows of the sorted model.

?
Internal model also maps columns to user roles as the
list only allows a single column.

[How to use List delegate](http://stackoverflow.com/questions/6905147/qt-qlistwidgetitem-multiple-lines).
[How to use List delegate](http://www.qtcentre.org/threads/27777-Customize-QListWidgetItem-how-to).


