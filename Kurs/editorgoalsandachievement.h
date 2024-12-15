#ifndef EDITORGOALSANDACHIEVEMENT_H
#define EDITORGOALSANDACHIEVEMENT_H

#include <QMainWindow>
#include <QListWidget>
#include <QMenu>
#include <QAction>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QWidget>
#include <QDir>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QDate>
#include <QLocale>
#include <QTimer>
#include <QTime>
#include <QCryptographicHash>

#include "customdialogmanager.h"
#include "customlistitem.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class EditorGoalsAndAchievement;
}
QT_END_NAMESPACE

class EditorGoalsAndAchievement : public QMainWindow
{
    Q_OBJECT

public:
    EditorGoalsAndAchievement(QWidget *parent = nullptr);
    ~EditorGoalsAndAchievement();

private slots:
    void on_addSphereOfLife_clicked();

    void on_addGoal_clicked();

    bool connectToDatabase();

    void onContextMenuSphere(const QPoint &pos);

    void onContextMenuElements(const QPoint &pos);

    bool deleteSphereOfLifeFromDb(const QString &name);

    bool deleteGoalsAndAchieveFromDb(int id);

    bool deleteGoalsAndAchieveFromDb(const QString &name, const QString &header);

    void setupGoalsAndAchieve();

    void loadSphereFromDb();

    void loadGoalsFromDb();

    void loadAchievementsFromDb();

    void loadPlannedFromDb();

    void showGoals(const QString &sphereName);

    void showAchievements(const QString &sphereName);

    void showPlanned(const QString &sphereName);

    void update();

    void onItemCheckStateChanged(QListWidgetItem *item);

    bool addGoalsToDb(QString name, QString date, QString sphere, int priority);

    bool addAchievementsToDb(QString name, QString date, QString sphere, int priority);

    bool addPlansToDb(QString name, QString date, QString sphere, int priority);

    void updateProgressBar(QListWidgetItem *current, QListWidgetItem *previous);

    void updateSphereCount(const QString &sphereName, int delta);

    double calculateProgressForSphere(const QString &sphereName) const;

    void on_sortList_clicked();

    void sortGoals(const QPair<QString, QString>& result);

    void sortAchievements(const QPair<QString, QString>& result);

    void sortPlans(const QPair<QString, QString>& result);

    void sortListByName(QList<QPair<QString, QPair<QString, QPair<QString, int>>>>& list, const QString& order);

    void sortListByDate(QList<QPair<QString, QPair<QString, QPair<QString, int>>>>& list, const QString& order);

    void sortListByPriority(QList<QPair<QString, QPair<QString, QPair<QString, int>>>>& list, const QString& order);

    void updateDatabaseAfterSort(const QString &table);

    void updateElapsedTime();

    QString hashPassword(const QString &password);

    void onEditGoalClicked(QListWidgetItem *item);

    void updateDatabaseAfterEdit(const QString &tableName,
                                const QPair<QString, QPair<QString, QPair<QString, int>>> &newGoal,
                                const QString &oldName,
                                const QString &oldSphere);

private:
    Ui::EditorGoalsAndAchievement *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = "/home/timofei/Coding/MyUniversityLabs/Third year/Visual programming and human-machine interaction Part 2/Kurs_Work/Kurs/Test";

    CustomDialogManager *customDialogManager;
    CustomListItem *customListItem;

    QList<QPair<QString, int>> spheres;
    QList<QPair<QString, QPair<QString, QPair<QString, int>>>> goals;
    QList<QPair<QString, QPair<QString, QPair<QString, int>>>> achievements;
    QList<QPair<QString, QPair<QString, QPair<QString, int>>>> plans;

    bool handleLogin(const QString &username, const QString &password);
    bool handleRegistration(const QString &username, const QString &password);


    QTimer *timer;
    QTime startTime;

    QPair<QPair<QString, QString>, bool> credentials;

    int userId;
};
#endif // EDITORGOALSANDACHIEVEMENT_H
