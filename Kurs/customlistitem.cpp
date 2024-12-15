#include "customlistitem.h"

CustomListItem::CustomListItem(QWidget *parent)
    : QWidget{parent}
{}

QListWidgetItem *CustomListItem::customItemGoal(const QString &name, const QString &date, const QString &sphere_name, int priority, Qt::CheckState checkState = Qt::Unchecked){
    QListWidgetItem *item = new QListWidgetItem();

    // Формируем текст элемента
    QString progressBar = QString("[") + QString(priority / 10, QChar('=')) + QString(10 - priority / 10, QChar('-')) + QString("]");
    QString itemText = QString("%1\t\tДата: %2\n\nПриоритет: %3 %4")
                           .arg(name)
                           .arg(date)
                           .arg(priority)
                           .arg(progressBar);

    // Устанавливаем текст
    item->setText(itemText);

    // Устанавливаем чекбокс
    item->setCheckState(checkState);                     // По умолчанию не отмечен

    // Сохраняем данные
    item->setData(Qt::UserRole, name);
    item->setData(Qt::UserRole + 1, date);
    item->setData(Qt::UserRole + 2, priority);
    item->setData(Qt::UserRole + 3, sphere_name);

    return item;

}

QListWidgetItem *CustomListItem::customItemSphere(const QString &name, int number){
    QString itemText = QString("%1\nЦелей: %2").arg(name).arg(number);

    // Создаем новый элемент списка
    QListWidgetItem *item = new QListWidgetItem();
    item->setText(itemText);  // Устанавливаем текст
    item->setData(Qt::UserRole, name);  // Сохраняем имя сферы жизни
    item->setData(Qt::UserRole+1, number);
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // item->setSizeHint(QSize(200, 60));  // Uncomment if needed with appropriate size

    return item;
}

QListWidgetItem *CustomListItem::сustomItemMain(const QString &name){
    QListWidgetItem *item = new QListWidgetItem();
    item->setText(name);
    item->setData(Qt::UserRole, name);
    item->setSizeHint(QSize(200, 60)); // Например, ширина: 200, высота: 50
    item->setTextAlignment(Qt::AlignVCenter);
    return item;

}
