#ifndef CUSTOMLISTITEM_H
#define CUSTOMLISTITEM_H

#include <QWidget>
#include <QListWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QCheckBox>
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QProgressBar>


class CustomListItem : public QWidget
{
    Q_OBJECT
public:
    explicit CustomListItem(QWidget *parent = nullptr);
    QListWidgetItem *customItemGoal(const QString &name, const QString &date, const QString &sphere_name, int priority, Qt::CheckState checkState);
    QListWidgetItem *customItemSphere(const QString &name, int number);
    QListWidgetItem *сustomItemMain(const QString &name);
    QListWidgetItem *customItemAchieve(const QString &name, const QString &date, const QString &sphere_name, int priority);

signals:
};

#endif // CUSTOMLISTITEM_H
