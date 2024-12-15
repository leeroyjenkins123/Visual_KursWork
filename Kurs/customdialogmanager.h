#ifndef CUSTOMDIALOGMANAGER_H
#define CUSTOMDIALOGMANAGER_H

#include <QObject>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>
#include <QString>
#include <QListWidget>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QSlider>
#include <QStackedWidget>
#include <QMessageBox>
#include <QTabWidget>

class CustomDialogManager : public QObject
{
    Q_OBJECT
public:
    explicit CustomDialogManager(QObject *parent = nullptr);
    QString showAddSphereOfLife(QWidget *parent);
    QPair<QString, QPair<QString, QPair<QString, int>>> showAddGoal(QWidget *parent, QListWidget *sphereOfLifeList);
    QPair<QString, QPair<QString, QPair<QString, int>>> showEditGoal(QWidget *parent, QListWidget *sphereOfLifeList, const QPair<QString, QPair<QString, QPair<QString, int>>>* currentGoal);
    QPair<QString, QString> showSortDialog(QWidget *parent);
    QPair<QPair<QString, QString>, bool> showLoginRegisterDialog(QWidget *parent);
signals:
};

#endif // CUSTOMDIALOGMANAGER_H
