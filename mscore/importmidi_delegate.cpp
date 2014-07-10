#include "importmidi_delegate.h"


namespace Ms {

SizedListWidget::SizedListWidget(QWidget *parent)
      : QListWidget(parent)
      {
      }

QSize SizedListWidget::sizeHint() const
      {
      const int extraHeight = 8;
      return QSize(width(), count() * visualItemRect(item(0)).height() + extraHeight);
      }

//----------------------------------------------------------------------------------

// values:
//    [0] == "__MultiValue__"
//  the rule for next values:
//    odd index - value to show
//    even index - "true" or "false" to define is the previous value checked or not

MultiValue::MultiValue(const QStringList &values, QWidget *parent)
      : QWidget(parent)
      {

      Q_ASSERT_X(values[0] == "__MultiValue__",
                 "Midi delegate - MultiValue class", "Invalid input values");

      QVBoxLayout *contentLayout = new QVBoxLayout();
      contentLayout->setSpacing(0);
      contentLayout->setContentsMargins(0, 0, 0, 0);

      _listWidget = new SizedListWidget();
      QListWidgetItem *allCheckBoxItem = new QListWidgetItem(QCoreApplication::translate(
                                                      "Multi value editor", "All"), _listWidget);
      allCheckBoxItem->setCheckState(Qt::Unchecked);
      _listWidget->addItem(allCheckBoxItem);
      contentLayout->addWidget(_listWidget);

      for (int i = 1; i < values.size() - 1; i += 2) {
            QListWidgetItem *item = new QListWidgetItem(values[i]);

            if (values[i + 1] == "true")
                  item->setCheckState(Qt::Checked);
            else if (values[i + 1] == "false")
                  item->setCheckState(Qt::Unchecked);
            else if (values[i + 1] == "undefined")
                  item->setCheckState(Qt::PartiallyChecked);
            else
                  Q_ASSERT_X(false, "Midi delegate - MultiValue class", "Unknown value");

            _listWidget->addItem(item);
            }

      setAllCheckBox();
      _states.resize(_listWidget->count());
      updateStates();

      connect(_listWidget, SIGNAL(itemClicked(QListWidgetItem *)),
              this, SLOT(itemClicked(QListWidgetItem *)));
      connectCheckBoxes();

      QHBoxLayout *buttonLayout = new QHBoxLayout();
      buttonLayout->addStretch();
      QPushButton *okButton = new QPushButton(QCoreApplication::translate(
                                                    "Multi value editor", "OK"));
      okButton->setMinimumWidth(50);
      okButton->setMinimumHeight(30);
      connect(okButton, SIGNAL(clicked()), SIGNAL(okClicked()));
      buttonLayout->addWidget(okButton);
      buttonLayout->addStretch();
      contentLayout->addLayout(buttonLayout);

      setLayout(contentLayout);

      setAutoFillBackground(true);
      }

QStringList MultiValue::data() const
      {
      QStringList values;
      for (int i = 1; i < _listWidget->count(); ++i) {
            switch (_listWidget->item(i)->checkState()) {
                  case Qt::Checked:
                        values.append("true");
                        break;
                  case Qt::Unchecked:
                        values.append("false");
                        break;
                  case Qt::PartiallyChecked:
                        values.append("undefined");
                        break;
                  }
            }
      return values;
      }

// The problem: we need to toggle the item state by clicking not only on the checkbox
// but on every point of the item for convenience;
// so if we clicked on the checkbox - we do not change the item state
// (because it is managed by the framework for us),
// if we clicked outside the checkbox - we change the state manually;
// after clicking on the checkbox the state is changed immediately, and after that
// the itemClicked signal is emitted; to detect this case we use the helper array
// of old checkbox states - if the item didn't change since the last time then the click
// was outside the checkbox and we set the state manually

void MultiValue::itemClicked(QListWidgetItem *item)
      {
      if (item->checkState() == _states[_listWidget->row(item)]) {      // clicked outside the checkbox
            disconnectCheckBoxes();
            switch (item->checkState()) {
                  case Qt::Checked:
                        item->setCheckState(Qt::Unchecked);
                        break;
                  case Qt::Unchecked:
                        item->setCheckState(Qt::Checked);
                        break;
                  case Qt::PartiallyChecked:
                        item->setCheckState(Qt::Checked);
                        break;
                  }
            connectCheckBoxes();
            checkBoxClicked(item);
            }
      updateStates();
      }

void MultiValue::checkBoxClicked(QListWidgetItem *item)
      {
      disconnectCheckBoxes();
      if (_listWidget->row(item) == 0) {           // "All" checkbox
            for (int i = 1; i < _listWidget->count(); ++i)
                  _listWidget->item(i)->setCheckState(_listWidget->item(0)->checkState());
            }
      else {
            setAllCheckBox();
            }
      connectCheckBoxes();
      }

void MultiValue::setAllCheckBox()
      {
      disconnectCheckBoxes();
      const auto firstValue = _listWidget->item(1)->checkState();
      bool wasSet = false;
      for (int i = 2; i < _listWidget->count(); ++i) {
            if (_listWidget->item(i)->checkState() != firstValue) {
                  _listWidget->item(0)->setCheckState(Qt::PartiallyChecked);
                  wasSet = true;
                  break;
                  }
            }
      if (!wasSet)
            _listWidget->item(0)->setCheckState(firstValue);
      connectCheckBoxes();
      }

void MultiValue::updateStates()
      {
      for (int i = 0; i < _listWidget->count(); ++i)
            _states[i] = _listWidget->item(i)->checkState();
      }

void MultiValue::connectCheckBoxes()
      {
      connect(_listWidget, SIGNAL(itemChanged(QListWidgetItem *)),
              this, SLOT(checkBoxClicked(QListWidgetItem *)));
      }

void MultiValue::disconnectCheckBoxes()
      {
      disconnect(_listWidget, SIGNAL(itemChanged(QListWidgetItem *)),
                 this, SLOT(checkBoxClicked(QListWidgetItem *)));
      }

//----------------------------------------------------------------------------------

OperationsDelegate::OperationsDelegate(QWidget *appWindow, bool rightArrowAlign)
      : appWindow(appWindow)
      , rightArrowAlign(rightArrowAlign)
      {}

void OperationsDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
      {
      QStyledItemDelegate::paint(painter, option, index);
                  // draw small arrow that symbolizes list
      QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList)
            {
            QStringList list = qvariant_cast<QStringList>(value);
            if (list.size() > 1)
                  drawArrow(painter, option, index);
            }
      }

void OperationsDelegate::drawArrow(
            QPainter *painter,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const
      {
      painter->save();

      QFontMetrics fm(painter->font());

      const int gap = 10;
      const int height = 4;
      const int width = 8;

      const int textWidth = fm.width(index.data(Qt::DisplayRole).toString());
      const int x = rightArrowAlign
                  ? option.rect.right() - width - gap
                  : option.rect.left() + textWidth + gap;
      const int y = option.rect.top() + option.rect.height() / 2 + 1;

      QPoint p1(x, y - height / 2);
      QPoint p2(x + width, y - height / 2);
      QPoint p3(x + width / 2, y + height / 2);

      QPen pen = painter->pen();
      painter->setPen(pen);
      pen.setWidth(1);
      painter->drawLine(p1, p2);
      pen.setWidth(2);
      painter->setPen(pen);
      painter->drawLine(QPoint(p2.x() - 1, p2.y() + 1), QPoint(p3.x() + 1, p3.y() - 1));
      pen.setWidth(1);
      painter->setPen(pen);
      painter->drawLine(p3, p1);

      painter->restore();
      }

QWidget* OperationsDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
      {
      const QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) {     // list of possible values
            const QStringList list = qvariant_cast<QStringList>(value);
            if (!list.isEmpty()) {
                  QWidget *editor = nullptr;

                  if (list[0] == "__MultiValue__") {
                        MultiValue *mv = new MultiValue(list, parent);
                        connect(mv, SIGNAL(okClicked()),
                                this, SLOT(commitAndCloseEditor()));
                        editor = mv;
                        }
                  else {
                        QListWidget *lw = new QListWidget(parent);
                        for (const auto &item: list)
                              lw->addItem(item);
                        connect(lw, SIGNAL(itemClicked(QListWidgetItem*)),
                                this, SLOT(commitAndCloseEditor()));
                        editor = lw;
                        }
                  return editor;
                  }
            }
                  // single value
      return QStyledItemDelegate::createEditor(parent, option, index);
      }

void OperationsDelegate::setEditorData(QWidget *editor,
                                       const QModelIndex &index) const
      {
      const QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) {
            QListWidget *lw = qobject_cast<QListWidget *>(editor);
            if (lw) {
                  const auto items = lw->findItems(index.data(Qt::DisplayRole).toString(), Qt::MatchExactly);
                  lw->setCurrentItem(items.empty() ? lw->item(0) : items.first());

                  const int extraWidth = 25;
                  const int extraHeight = 6;
                  lw->setMinimumWidth(lw->sizeHintForColumn(0) + extraWidth);

                              // to prevent possible hiding bottom part of the list
                  const int h = lw->count() * (lw->visualItemRect(lw->currentItem()).height() + extraHeight);
                  const int y = (lw->parentWidget() && (lw->parentWidget()->rect().bottom() < lw->y() + h))
                              ? lw->parentWidget()->rect().bottom() - h - extraHeight : lw->y();
                  lw->setGeometry(lw->x(), y, lw->width(), h);

                              // now lw can be partially hidden behind the view
                              // if the view has small rect, so set parent of lw
                              // to app window and map coordinates accordingly to leave lw in place
                  const auto globalCoord = lw->parentWidget()->mapToGlobal(lw->geometry().topLeft());
                  lw->setParent(appWindow);
                  const auto newLocalCoord = appWindow->mapFromGlobal(globalCoord);
                  lw->setGeometry(newLocalCoord.x(), newLocalCoord.y(), lw->width(), h);
                  }
            else {
                  MultiValue *mv = qobject_cast<MultiValue *>(editor);

                  Q_ASSERT_X(mv, "Midi delegate - setEditorData", "Unknown editor type");

                              // to prevent possible hiding bottom part of the checkbox list
                  const int h = mv->sizeHint().height();
                  const int y = (mv->parentWidget() && (mv->parentWidget()->rect().bottom() < mv->y() + h))
                              ? mv->parentWidget()->rect().bottom() - h : mv->y();
                  mv->setGeometry(mv->x(), y, mv->width(), h);

                              // now mv can be partially hidden behind the view
                              // if the view has small rect, so set parent of mv
                              // to app window and map coordinates accordingly to leave mv in place
                  const auto globalCoord = mv->parentWidget()->mapToGlobal(mv->geometry().topLeft());
                  mv->setParent(appWindow);
                  const auto newLocalCoord = appWindow->mapFromGlobal(globalCoord);
                  mv->setGeometry(newLocalCoord.x(), newLocalCoord.y(), mv->width(), h);
                  }
            }
      else {       // single value
            QStyledItemDelegate::setEditorData(editor, index);
            }
      }

void OperationsDelegate::setModelData(QWidget *editor,
                                      QAbstractItemModel *model,
                                      const QModelIndex &index) const
      {
      const QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) {
            const QListWidget *lw = qobject_cast<QListWidget *>(editor);
            if (lw) {
                  model->setData(index, lw->currentRow());
                  }
            else {
                  const MultiValue *mv = qobject_cast<MultiValue *>(editor);

                  Q_ASSERT_X(mv, "Midi delegate - setModelData", "Unknown editor type");

                  model->setData(index, mv->data());
                  }
            }
      else {
            QStyledItemDelegate::setModelData(editor, model, index);
            }
      }

void OperationsDelegate::commitAndCloseEditor()
      {
      QWidget *editor = qobject_cast<QWidget *>(sender());
      emit commitData(editor);
      emit closeEditor(editor);
      editor->parentWidget()->setFocus();
      }

} // namespace Ms
