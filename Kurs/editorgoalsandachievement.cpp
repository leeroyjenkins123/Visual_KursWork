#include "editorgoalsandachievement.h"
#include "./ui_editorgoalsandachievement.h"
#include <QApplication>

EditorGoalsAndAchievement::EditorGoalsAndAchievement(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EditorGoalsAndAchievement)
    ,customDialogManager(new CustomDialogManager(this))
    ,customListItem(new CustomListItem(this))
    , timer(new QTimer(this))
{
    ui->setupUi(this);

    if (!connectToDatabase()) {
        QMessageBox::critical(this, "Error", "Cannot connect to database.");
        return;
    }

    credentials = customDialogManager->showLoginRegisterDialog(this);

    // Проверка, если пользователь отменил вход или регистрацию
    if (credentials.first.first.isEmpty() || credentials.first.second.isEmpty()) {
        QMessageBox::information(this, "Закрытие", "Вход или регистрация отменены. Программа будет закрыта.");
        this->setVisible(false);
        return;
    }
    else{
        QString login = credentials.first.first;
        QString password = credentials.first.second;
        bool isLogin = credentials.second;

        if (isLogin) {
            // Если флаг входа
            if (!handleLogin(login, password)) {
                QMessageBox::critical(this, "Ошибка", "Не удалось выполнить вход.");
                this->setVisible(false);
                return;
            }
        } else {
            // Если флаг регистрации
            if (!handleRegistration(login, password)) {
                QMessageBox::critical(this, "Ошибка", "Не удалось выполнить регистрацию.");
                this->setVisible(false);
                return;
            }
        }
    }
    this->setVisible(true);
    startTime = QTime::currentTime();

    qDebug()<<"UserId: "<<userId;

    ui->progressBar->hide();

    connect(timer, &QTimer::timeout, this, &EditorGoalsAndAchievement::updateElapsedTime);

    QLocale russianLocale(QLocale::Russian, QLocale::Russia);
    QString currentDate = russianLocale.toString(QDate::currentDate(), "dddd, d MMMM");
    currentDate[0] = currentDate[0].toUpper(); // Делаем первую букву заглавной
    ui->dateHeader->setText(currentDate);

    ui->listGoalsAndAchieve->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listGoalsAndAchieve, &QListWidget::customContextMenuRequested, this, &EditorGoalsAndAchievement::onContextMenuElements);

    ui->sphereOfLife->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->sphereOfLife, &QListWidget::customContextMenuRequested, this, &EditorGoalsAndAchievement::onContextMenuSphere);

    connect(ui->listGoalsAndAchieve, &QListWidget::itemChanged, this, &EditorGoalsAndAchievement::onItemCheckStateChanged);

    connect(ui->listGoalsAndAchieve, &QListWidget::itemClicked, this, &EditorGoalsAndAchievement::onEditGoalClicked);

    connect(ui->goalsAndAchieve, &QListWidget::currentItemChanged, [=](QListWidgetItem *current, QListWidgetItem *previous){
        if(current){
            QString text = current->data(Qt::UserRole).toString();
            ui->mainHeader->setText(text);

            QListWidgetItem *currentSphere = ui->sphereOfLife->currentItem();
            QString sphereName;
            if(currentSphere){
                sphereName = currentSphere->data(Qt::UserRole).toString();
            }

            qDebug() << "Current Item in goalsAndAchieve:" << text;
            qDebug() << "Current Sphere:" << sphereName;

            if (text == "Цели") {
                showGoals(sphereName); // Передаем сферу жизни
                ui->progressBar->hide();
            } else if (text == "Достижения") {
                showAchievements(sphereName); // Передаем сферу жизни
                if (currentSphere) {
                    updateProgressBar(currentSphere, nullptr);
                } else {
                    ui->progressBar->setValue(0); // Set to 0 if no sphere is selected
                    ui->progressBar->show();
                }
            } else if (text == "Запланировано") {
                showPlanned(sphereName); // Передаем сферу жизни
                ui->progressBar->hide();
            }
        }
        else{
            ui->mainHeader->clear();
        }
    });

    connect(ui->sphereOfLife, &QListWidget::currentItemChanged, [=](QListWidgetItem *current, QListWidgetItem *previous) {
        if (current) {
            QString sphereText = current->data(Qt::UserRole).toString();
            ui->sphereHeader->setText("\"" + sphereText + "\"");

            // Получаем выбранный пункт в goalsAndAcieve
            QListWidgetItem *currentGoal = ui->goalsAndAchieve->currentItem();
            if (currentGoal) {
                QString goalsText = currentGoal->data(Qt::UserRole).toString();

                qDebug() << "Selected Sphere:" << sphereText;
                qDebug() << "Current Goals Text:" << goalsText;

                // Обновляем содержимое в зависимости от новой сферы жизни
                if (goalsText == "Цели") {
                    ui->progressBar->hide();
                    showGoals(sphereText);
                } else if (goalsText == "Достижения") {
                    showAchievements(sphereText);
                    updateProgressBar(current, previous);
                } else if (goalsText == "Запланировано") {
                    ui->progressBar->hide();
                    showPlanned(sphereText);
                }
            }
        } else {
            ui->sphereHeader->clear();
        }
    });

    setupGoalsAndAchieve();

    loadSphereFromDb();
    loadGoalsFromDb();
    loadPlannedFromDb();
    loadAchievementsFromDb();

    timer->start(1000);
}

EditorGoalsAndAchievement::~EditorGoalsAndAchievement()
{
    timer->stop();
    delete timer;
    delete ui;
    delete customDialogManager;
    delete customListItem;
    db.close();
    QSqlDatabase::removeDatabase(dbPath);
}

void EditorGoalsAndAchievement::updateElapsedTime()
{
    QTime elapsedTime = QTime::currentTime().addMSecs(-startTime.msecsSinceStartOfDay());
    QString timeString = elapsedTime.toString("hh:mm:ss");
    ui->timerLabel->setText("Elapsed Time: " + timeString);
}

void EditorGoalsAndAchievement::on_addSphereOfLife_clicked()
{
    QString sphereName = customDialogManager->showAddSphereOfLife(this);
    QListWidgetItem *item = customListItem->customItemSphere(sphereName, 0);
    if (!sphereName.isEmpty()) {
        ui->sphereOfLife->addItem(item);
        qDebug() << "Sphere name: " << sphereName;
        spheres.append(qMakePair(sphereName, 0));

        // Подготовка запроса с использованием связывания параметров для безопасности
        QSqlQuery query;
        query.prepare("INSERT INTO sphere_of_life (name, user_id) VALUES (:name, :user_id)");
        query.bindValue(":name", sphereName);
        query.bindValue(":user_id", userId);

        qDebug() << "Executed query: " << query.lastQuery();


        if (!query.exec()) {
            qDebug() << "Ошибка при добавлении сферы жизни в базу данных:" << query.lastError().text();
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить сферу жизни в базу данных.\n" + query.lastError().text());
        } else {
            qDebug() << "Сфера жизни добавлена успешно.";
        }
    }
}


void EditorGoalsAndAchievement::on_addGoal_clicked()
{
    if(ui->sphereOfLife->count()<1){
        QMessageBox::critical(this, "Ошибка", "У вас нет ни одной сферы для добавления цели");
        return;
    }

    QPair<QString, QPair<QString, QPair<QString, int>>> result = customDialogManager->showAddGoal(this, ui->sphereOfLife);

    if(result.first.isEmpty()){
        return;
    }

    QString name = result.first;
    QString date = result.second.first;

    QString sphere = result.second.second.first;
    int priority = result.second.second.second;

    if (!sphere.isEmpty()) {
        updateSphereCount(sphere, 1);
    }

    updateProgressBar(ui->sphereOfLife->currentItem(), nullptr);

    qDebug()<<name<<" "<<date<<" "<<sphere<<" "<<priority;

    if (date.isEmpty()) {
        qDebug() << "Дата не может быть пустой!";
        QMessageBox::critical(this, "Ошибка", "Дата не может быть пустой!");
        return; // Прерываем выполнение функции
    }

    QDate goalDate = QDate::fromString(date, "dd.MM.yyyy");
    QDate currentDate = QDate::currentDate();

    if(goalDate < currentDate){
        qDebug() << "Дата не может быть меньше текущей!";
        QMessageBox::critical(this, "Ошибка", "Дата не может быть меньше текущей!");
        return; // Прерываем выполнение функции
    }

    if(goalDate == currentDate){
        goals.append(result);
        if (!addGoalsToDb(name,date,sphere,priority)) {
            qDebug() << "Ошибка при добавлении цели в базу данных";
        } else {
            qDebug() << "Текущая цель записана.";
            QListWidgetItem *currentSphere = ui->sphereOfLife->currentItem();
            QString currentSphereName;
            if (currentSphere) {
                currentSphereName = currentSphere->data(Qt::UserRole).toString();
            }

            // Проверяем, выбран ли пункт "Цели" в goalsAndAchieve
            QListWidgetItem *currentGoal = ui->goalsAndAchieve->currentItem();
            if (currentGoal && currentGoal->data(Qt::UserRole).toString() == "Цели") {
                showGoals(currentSphereName);
            }
        }
    }
    else{
        plans.append(result);
        if (!addPlansToDb(name,date,sphere,priority)) {
            qDebug() << "Ошибка при добавлении планированной цели в базу данных:";
            // QMessageBox::critical(this, "Ошибка", "Не удалось добавить сферу жизни в базу данных.\n" + query.lastError().text());
        } else {
            qDebug() << "Планируемая цель записана.";
            QListWidgetItem *currentSphere = ui->sphereOfLife->currentItem();
            QString currentSphereName;
            if (currentSphere) {
                currentSphereName = currentSphere->data(Qt::UserRole).toString();
            }

            // Проверяем, выбран ли пункт "Цели" в goalsAndAchieve
            QListWidgetItem *currentGoal = ui->goalsAndAchieve->currentItem();
            if (currentGoal && currentGoal->data(Qt::UserRole).toString() == "Цели") {
                showPlanned(currentSphereName);
            }
        }
    }
}

bool EditorGoalsAndAchievement::connectToDatabase(){
    db.setDatabaseName(dbPath);

    if (!db.isOpen()) {
        if(!db.open()){
            qDebug() << "Database connection failed:" << db.lastError().text();
            return false;
        }
    } else {
        qDebug() << "Database connected successfully!";
    }

    return true;
}

void EditorGoalsAndAchievement::setupGoalsAndAchieve(){
    QList<QString> main = {"Цели", "Достижения", "Запланировано"};
    for(int i = 0; i < main.count(); i++){
        QListWidgetItem *item1 = customListItem->сustomItemMain(main[i]);
        ui->goalsAndAchieve->addItem(item1);
    }

    // Clear the main header
    for (int i = 0; i < ui->goalsAndAchieve->count(); ++i) {
        QListWidgetItem *item = ui->goalsAndAchieve->item(i);
        item->setTextAlignment(Qt::AlignCenter);
    }
    ui->mainHeader->clear();
}

void EditorGoalsAndAchievement::loadSphereFromDb(){
    spheres.clear();
    // ui->sphereOfLife->clear();

    QSqlQuery query("SELECT name, count FROM sphere_of_life WHERE user_id = ?");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            QString sphereName = query.value(0).toString();  // Извлекаем имя
            int quantity = query.value(1).toInt();  // Извлекаем количество

            spheres.append(qMakePair(sphereName, quantity));

            QListWidgetItem *item = customListItem->customItemSphere(sphereName, quantity);
            ui->sphereOfLife->addItem(item);
        }
    } else {
        qDebug() << "Error load Sphere executing query:" << query.lastError().text();
    }

    ui->sphereHeader->clear();


    ui->sphereOfLife->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->sphereOfLife->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->sphereOfLife->setViewMode(QListView::ListMode);
    ui->sphereOfLife->setFlow(QListView::TopToBottom);
}
void EditorGoalsAndAchievement::loadGoalsFromDb() {
    goals.clear();
    QSqlQuery query;
    query.prepare("SELECT g.name, g.date, g.priority, s.name "
                  "FROM goals g "
                  "INNER JOIN sphere_of_life s ON g.sphere_of_life_id = s.id "
                  "WHERE g.user_id = ?");
    query.bindValue(0, userId); // Ensure userId is an integer

    if (query.exec()) {
        while (query.next()) {
            QString name = query.value(0).toString();
            QString dueDate = query.value(1).toString();
            int priority = query.value(2).toInt();
            QString sphereName = query.value(3).toString();

            goals.append(qMakePair(name, qMakePair(dueDate, qMakePair(sphereName, priority))));
        }
    } else {
        qDebug() << "Error load goals executing query:" << query.lastError().text();
    }
}

void EditorGoalsAndAchievement::loadAchievementsFromDb(){
    achievements.clear();
    QSqlQuery query;
    query.prepare("SELECT a.name, a.achievement_date, a.priority, s.name "
                  "FROM achievements a "
                  "INNER JOIN sphere_of_life s ON a.sphere_of_life_id = s.id "
                  "WHERE a.user_id = ?");
    query.bindValue(0, userId);
    if (query.exec()) {
        while (query.next()) {
            QString name = query.value(0).toString();
            QString achievement_date = query.value(1).toString();
            int priority = query.value(2).toInt();
            QString sphereName = query.value(3).toString();

            qDebug() << "Name:" << name << "Date:" << achievement_date << "Sphere:" << sphereName << "Priority:" << priority;

            achievements.append(qMakePair(name, qMakePair(achievement_date, qMakePair(sphereName, priority))));
        }
    } else {
        qDebug() << "Error load achieve executing query:" << query.lastError().text();
    }

    qDebug() << "Achievements count:" << achievements.count();
    for (const auto &achievement : achievements) {
        qDebug() << achievement.first << achievement.second.first << achievement.second.second.first << achievement.second.second.second;
    }
}

void EditorGoalsAndAchievement::loadPlannedFromDb(){
    achievements.clear();
    QSqlQuery query;
    query.prepare("SELECT p.name, p.planned_date, p.priority, s.name "
                  "FROM planned_goals p "
                  "INNER JOIN sphere_of_life s ON p.sphere_of_life_id = s.id "
                  "WHERE p.user_id = ?");
    query.bindValue(0, userId);
    QDate goalDate;
    QDate currentDate = QDate::currentDate();
    if (query.exec()) {
        while (query.next()) {
            QString name = query.value(0).toString();
            QString planned_date = query.value(1).toString();
            int priority = query.value(2).toInt();
            QString sphere_name = query.value(3).toString();

            goalDate = QDate::fromString(planned_date, "dd.MM.yyyy");

            if(goalDate <= currentDate){
                goals.append(qMakePair(name, qMakePair(planned_date, qMakePair(sphere_name, priority))));
            }
            else{
                plans.append(qMakePair(name, qMakePair(planned_date, qMakePair(sphere_name, priority))));
            }

        }
    } else {
        qDebug() << "Error load planned goals executing query:" << query.lastError().text();
    }
}

void EditorGoalsAndAchievement::showGoals(const QString &sphereName){
    ui->listGoalsAndAchieve->clear();
    if(goals.count()>0){
        for(int i=0;i<goals.count();i++){
            if(goals[i].second.second.first == sphereName){
                QListWidgetItem *item = customListItem->customItemGoal(goals[i].first, goals[i].second.first, sphereName, goals[i].second.second.second, Qt::Unchecked);
                ui->listGoalsAndAchieve->addItem(item);
            }
        }
    }
   ui->listGoalsAndAchieve->update();
}

void EditorGoalsAndAchievement::showAchievements(const QString &sphereName){
    ui->listGoalsAndAchieve->clear();
    qDebug() << "Achievements count to show:" << achievements.count();
    if(achievements.count() > 0){
        for(int i = 0; i < achievements.count(); i++) {
            // Логируем данные перед фильтрацией
            qDebug() << "Filtering achievements for sphere:" << sphereName;
            qDebug() << "Achievement sphere:" << achievements[i].second.second.first;

            // Фильтруем по названию сферы
            if (achievements[i].second.second.first == sphereName) {
                qDebug() << "Adding achievement: "
                         << achievements[i].first
                         << achievements[i].second.first
                         << sphereName
                         << achievements[i].second.second.second;

                // Создаем элемент для списка
                QListWidgetItem *item = customListItem->customItemGoal(
                    achievements[i].first,
                    achievements[i].second.first,
                    sphereName,
                    achievements[i].second.second.second,
                    Qt::Checked
                    );

                // Добавляем элемент в список
                ui->listGoalsAndAchieve->addItem(item);
            }
        }
    } else {
        qDebug() << "No achievements to display.";
    }
    ui->listGoalsAndAchieve->update();
}

void EditorGoalsAndAchievement::showPlanned(const QString &sphereName){
    ui->listGoalsAndAchieve->clear();
    if(plans.count()>0){
        for(int i=0;i<plans.count();i++){
            if(plans[i].second.second.first == sphereName){
                QListWidgetItem *item = customListItem->customItemGoal(plans[i].first, plans[i].second.first, sphereName, plans[i].second.second.second, Qt::Unchecked);
                ui->listGoalsAndAchieve->addItem(item);
            }
        }
    }
    ui->listGoalsAndAchieve->update();
}

void EditorGoalsAndAchievement::onContextMenuSphere(const QPoint &pos){
    QString text;
    QListWidgetItem *item = ui->sphereOfLife->itemAt(pos); // Элемент по позиции клика
    if (!item) return;

    text = item->data(Qt::UserRole).toString();

    // Создаем контекстное меню
    QMenu contextMenu(this);
    QAction *deleteAction = new QAction("Удалить", &contextMenu);

    // Добавляем действие удаления в меню
    contextMenu.addAction(deleteAction);

    // Подключаем обработчик действия
    connect(deleteAction, &QAction::triggered, [this, item, text]() {
        // Удаляем выбранный элемент
        if (deleteSphereOfLifeFromDb(text)) {

            // Удаляем элемент из интерфейса
            delete item;
            if (ui->sphereOfLife->count() > 0) {
                ui->sphereOfLife->setCurrentRow(0); // Устанавливаем первый элемент текущим
            } else {
                ui->sphereHeader->clear();
                // Additional UI updates if necessary
            }
            QMessageBox::information(this, "Успех", "Сфера жизни успешно удалена!");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить сферу жизни из базы данных.");
        }
    });

    // Показываем меню
    contextMenu.exec(ui->sphereOfLife->mapToGlobal(pos));
}

void EditorGoalsAndAchievement::onContextMenuElements(const QPoint &pos){
    QString text;
    QListWidgetItem *item = ui->listGoalsAndAchieve->itemAt(pos); // Элемент по позиции клика
    if (!item) return;

    text = item->data(Qt::UserRole).toString();
    QString header = ui->mainHeader->text();

    // Создаем контекстное меню
    QMenu contextMenu(this);
    QAction *deleteAction = new QAction("Удалить", &contextMenu);

    // Добавляем действие удаления в меню
    contextMenu.addAction(deleteAction);

    // Подключаем обработчик действия
    connect(deleteAction, &QAction::triggered, [this, item, text, header]() {
        // Удаляем выбранный элемент
        if (deleteGoalsAndAchieveFromDb(text, header)) {
            // Удаляем элемент из интерфейса
            delete item;
            if (ui->listGoalsAndAchieve->count() > 0) {
                ui->listGoalsAndAchieve->setCurrentRow(0); // Устанавливаем первый элемент текущим
            }
            QMessageBox::information(this, "Успех", "Сфера жизни успешно удалена!");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить сферу жизни из базы данных.");
        }
    });

    // Показываем меню
    contextMenu.exec(ui->sphereOfLife->mapToGlobal(pos));
}

bool EditorGoalsAndAchievement::deleteSphereOfLifeFromDb(const QString &name)
{
    if (!db.isOpen()) {
        qDebug() << "Ошибка: База данных не открыта.";
        return false;
    }

    int id=0;

    // Подготавливаем запрос для удаления записи по имени
    QSqlQuery query;
    query.prepare("SELECT id FROM sphere_of_life WHERE name = (:name) AND user_id = :user_id");
    query.bindValue(":name", name);
    query.bindValue(":user_id", userId);

    if(query.exec()){
        id = query.value(0).toInt();
    }

    query.prepare("DELETE FROM sphere_of_life WHERE name = (:name) AND user_id = :user_id");
    query.bindValue(":name", name);
    query.bindValue(":user_id", userId);

    // Выполняем запрос
    if (!query.exec()) {
        qDebug() << "Ошибка при удалении из базы данных:" << query.lastError().text();
        return false;
    }

    deleteGoalsAndAchieveFromDb(id);

    return true;
}

bool EditorGoalsAndAchievement::deleteGoalsAndAchieveFromDb(int id){
    if (!db.isOpen()) {
        qDebug() << "Ошибка: База данных не открыта.";
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM goals WHERE sphere_of_life_id = :id AND user_id = :user_id");
    query.bindValue(":id", id);
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка при удалении из таблицы goals:" << query.lastError().text();
        return false;
    }

    query.prepare("DELETE FROM achievements WHERE sphere_of_life_id = :id AND user_id = :user_id");
    query.bindValue(":id", id);
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка при удалении из таблицы achievements:" << query.lastError().text();
        return false;
    }

    query.prepare("DELETE FROM planned_goals WHERE sphere_of_life_id = :id AND user_id = :user_id");
    query.bindValue(":id", id);
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка при удалении из таблицы planned:" << query.lastError().text();
        return false;
    }

    return true;
}

bool EditorGoalsAndAchievement::deleteGoalsAndAchieveFromDb(const QString &name, const QString &header){
    if (!db.isOpen()) {
        qDebug() << "Ошибка: База данных не открыта.";
        return false;
    }
    QString table;
    QSqlQuery query;
    if(header == "Цели"){
        table = "goals";
        for(int i=0;i<goals.count();i++){
            if(goals[i].first == name){
                updateSphereCount(goals[i].second.second.first, -1);
                goals.removeAt(i);
                break;
            }
        }
        query.prepare(QString("DELETE FROM goals WHERE name = :name AND user_id = :user_id"));
        query.bindValue(":user_id", userId);

    }
    else if(header == "Достижения"){
        table = "achievements";
        for(int i=0;i<achievements.count();i++){
            if(achievements[i].first == name){
                updateSphereCount(achievements[i].second.second.first, -1);
                achievements.removeAt(i);
                break;
            }
        }
        query.prepare(QString("DELETE FROM achievements WHERE name = :name AND user_id = :user_id"));
        query.bindValue(":user_id", userId);
    }

    else{
        table = "planned_goals";
        for(int i=0;i<plans.count();i++){
            if(plans[i].first == name){
                updateSphereCount(plans[i].second.second.first, -1);
                plans.removeAt(i);
                break;
            }
        }
        query.prepare(QString("DELETE FROM planned_goals WHERE name = :name AND user_id = :user_id"));
        query.bindValue(":user_id", userId);
    }



    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << "Ошибка при удалении из таблицы planned:" << query.lastError().text();
        return false;
    }

    return true;
}

void EditorGoalsAndAchievement::update(){
    showGoals(ui->sphereHeader->text());
}

void EditorGoalsAndAchievement::onItemCheckStateChanged(QListWidgetItem *item) {
    // Получаем имя цели из данных элемента
    QString goalName = item->data(Qt::UserRole).toString();
    if (goalName.isEmpty()) {
        qDebug() << "Goal name is empty.";
        return;
    }

    QList<QPair<QString, QPair<QString, QPair<QString, int>>>> *sourceList = nullptr;
    QList<QPair<QString, QPair<QString, QPair<QString, int>>>> *targetList = nullptr;

    // Определяем, из какого списка элемент и в какой он должен переместиться
    if (ui->mainHeader->text() == "Цели") {
        sourceList = &goals;
        targetList = &achievements;
    } else if (ui->mainHeader->text() == "Запланировано") {
        sourceList = &plans;
        targetList = &achievements;
    } else if (ui->mainHeader->text() == "Достижения") {
        sourceList = &achievements;
    } else {
        qDebug() << "Unknown main header: " << ui->mainHeader->text();
        return;
    }

    // Найти индекс элемента в исходном списке
    int goalIndex = -1;
    if (sourceList) {
        for (int i = 0; i < sourceList->size(); ++i) {
            if ((*sourceList)[i].first == goalName) {
                goalIndex = i;
                break;
            }
        }
    }

    if (goalIndex == -1) {
        qDebug() << "Goal not found in the source list: " << goalName;
        return;
    }

    // Перемещение элемента между списками
    auto goal = sourceList->takeAt(goalIndex);
    ui->listGoalsAndAchieve->takeItem(ui->listGoalsAndAchieve->row(item));

    qDebug()<<item->checkState();

    if (item->checkState() == Qt::Checked && targetList) {
        // Переместить из Goals/Plans в Achievements
        qDebug()<<goal;
        targetList->append(goal);
        if (!addAchievementsToDb(goal.first, goal.second.first, goal.second.second.first, goal.second.second.second)) {
            qDebug() << "Error adding achievement to database.";
        }
        deleteGoalsAndAchieveFromDb(goal.first, ui->mainHeader->text());
        updateSphereCount(goal.second.second.first, -1);
        item->setCheckState(Qt::Checked);

        // QListWidgetItem *item = customListItem->customItemGoal(goal.first, goal.second.first, goal.second.second.first, goal.second.second.second, Qt::Checked);
        // ui->listGoalsAndAchieve->addItem(item);
    } else if (item->checkState() == Qt::Unchecked && !targetList) {
        // Вернуть из Achievements в Goals или Plans
        QDate currentDate = QDate::currentDate();
        QDate dueDate = QDate::fromString(goal.second.first, "dd.MM.yyyy");

        if (currentDate > dueDate) {
            plans.append(goal);
            addPlansToDb(goal.first, goal.second.first, goal.second.second.first, goal.second.second.second);
        } else {
            goals.append(goal);
            addGoalsToDb(goal.first, goal.second.first, goal.second.second.first, goal.second.second.second);
        }

        QSqlQuery query;
        query.prepare("DELETE FROM achievements WHERE name = ? AND user_id = ?");
        query.bindValue(0, goal.first);
        query.bindValue(1, userId);
        if (!query.exec()) {
            qDebug() << "Error removing achievement from database:" << query.lastError().text();
        } else {
            qDebug() << "Achievement removed from database.";
        }
        updateSphereCount(goal.second.second.first, 1);
        updateProgressBar(ui->sphereOfLife->currentItem(), nullptr);
        item->setCheckState(Qt::Unchecked);
        // QListWidgetItem *item = customListItem->customItemGoal(goal.first, goal.second.first, goal.second.second.first, goal.second.second.second, Qt::Unchecked);
        // ui->listGoalsAndAchieve->addItem(item);
    }
}

bool EditorGoalsAndAchievement::addGoalsToDb(QString name, QString date, QString sphere, int priority) {
    QSqlQuery querySphere;
    querySphere.prepare("SELECT id FROM sphere_of_life WHERE name = :sphere_name");
    querySphere.bindValue(":sphere_name", sphere);
    if (!querySphere.exec()) {
        qDebug() << "Error fetching sphere id:" << querySphere.lastError().text();
        return false;
    }
    if (querySphere.next()) {
        int sphereId = querySphere.value(0).toInt();
        qDebug()<<"SphereId for add goal: "<<sphereId;
        QSqlQuery queryInsert;
        queryInsert.prepare("INSERT INTO goals (name, date, priority, sphere_of_life_id, user_id) "
                            "VALUES (:name, :date, :priority, :sphere_id, :user_id)");
        queryInsert.bindValue(":name", name);
        queryInsert.bindValue(":date", date);
        queryInsert.bindValue(":priority", priority);
        queryInsert.bindValue(":sphere_id", sphereId);
        queryInsert.bindValue(":user_id", userId); // Ensure 'userId' is defined
        if (!queryInsert.exec()) {
            qDebug() << "Error adding goal to database in addGoalsToDb:" << queryInsert.lastError().text();
            return false;
        }
        return true;
    } else {
        qDebug() << "Sphere not found:" << sphere;
        return false;
    }
}

bool EditorGoalsAndAchievement::addAchievementsToDb(QString name, QString date, QString sphere, int priority){
    QSqlQuery query;
    query.prepare("INSERT INTO achievements (name, achievement_date, priority, user_id, sphere_of_life_id)"
                  "VALUES (:name, :achievement_date, :priority,  :user_id, (SELECT id FROM sphere_of_life WHERE name = :sphere_of_life_name))");

    query.bindValue(":name", name);
    query.bindValue(":achievement_date", date);
    query.bindValue(":priority", priority);
    query.bindValue(":sphere_of_life_name", sphere);
    query.bindValue(":user_id", userId);

    if(!query.exec()){
        return false;
    }

    return true;
}

bool EditorGoalsAndAchievement::addPlansToDb(QString name, QString date, QString sphere, int priority){
    QSqlQuery query;
    query.prepare("INSERT INTO planned_goals (name, planned_date, priority, sphere_of_life_id, user_id)"
                  "VALUES (:name, :planned_date, :priority, (SELECT id FROM sphere_of_life WHERE name = :sphere_of_life_name), :user_id)");

    query.bindValue(":name", name);
    query.bindValue(":planned_date", date);
    query.bindValue(":priority", priority);
    query.bindValue(":sphere_of_life_name", sphere);
    query.bindValue(":user_id", userId);

    if(!query.exec()){
        return false;
    }
    return true;
}

void EditorGoalsAndAchievement::updateProgressBar(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous);

    if (current) {
        QString sphereName = current->data(Qt::UserRole).toString();
        qDebug()<<"SphereName for progressBar "<<sphereName;
        double progress = calculateProgressForSphere(sphereName);
        ui->progressBar->setValue(static_cast<int>(progress));
    }
    else {
        if (ui->mainHeader->text() == "Достижения") {
            ui->progressBar->setValue(0);
            ui->progressBar->show();
        } else {
            ui->progressBar->hide();
        }
    }

    if(ui->mainHeader->text() != "Достижения"){
        ui->progressBar->hide();
    }
    else{
        ui->progressBar->show();
    }
}

double EditorGoalsAndAchievement::calculateProgressForSphere(const QString &sphereName) const {
    int total = 0;
    int completed = 0;

    qDebug()<<"SphereName in calculateProgressForSphere"<<sphereName;

    // Count goals for the sphere
    for (const auto &goal : goals) {
        if (goal.second.second.first == sphereName) {
            total++;
        }
    }

    // Count planned goals for the sphere
    for (const auto &plan : plans) {
        if (plan.second.second.first == sphereName) {
            total++;
        }
    }

    // Count achievements for the sphere
    for (const auto &achievement : achievements) {
        if (achievement.second.second.first == sphereName) {
            completed++;
        }
    }

    total += completed;

    if (total == 0 && completed > 0) {
        return 100.0;
    }
    else if(total == 0 && completed == 0){
        return 0.0;
    }

    return (static_cast<double>(completed) / total) * 100.0;
}

void EditorGoalsAndAchievement::updateSphereCount(const QString &sphereName, int delta) {
    for (auto &sphere : spheres) {
        if (sphere.first == sphereName) {
            // Обновляем значение счетчика в памяти
            sphere.second = sphere.second + delta;

            if (sphere.second < 0) {
                QMessageBox::critical(this, "Ошибка", "Не может быть отрицательное количество целей.");
                return;
            }

            // Обновляем элемент в UI
            for (int i = 0; i < ui->sphereOfLife->count(); ++i) {
                QListWidgetItem *item = ui->sphereOfLife->item(i);
                QString sphereText = item->data(Qt::UserRole).toString();

                if (sphereText == sphereName) {
                    // Обновляем количество на UI
                    item->setText(customListItem->customItemSphere(sphereName, sphere.second)->text());
                    // item->setText(QString("%1\t%2").arg(sphereName).arg(sphere.second));
                    break;
                }
            }

            // Обновляем значение в базе данных
            QSqlQuery query;
            query.prepare("UPDATE sphere_of_life SET count = :count WHERE name = :name AND user_id = :user_id");
            query.bindValue(":count", sphere.second);
            query.bindValue(":name", sphereName);
            query.bindValue(":user_id", userId);

            if (!query.exec()) {
                qDebug() << "Ошибка при обновлении количества в базе данных:" << query.lastError().text();
            } else {
                qDebug() << "Количество в базе данных обновлено успешно.";
            }

            break;
        }
    }
}

void EditorGoalsAndAchievement::on_sortList_clicked()
{
    bool hasSelectedSphere = (ui->sphereOfLife->currentItem() != nullptr);
    bool hasAtLeastTwoItems = (ui->listGoalsAndAchieve->count() >= 2);
    if(!hasSelectedSphere || !hasAtLeastTwoItems){
        QMessageBox::warning(this, "Недостаточно элементов", "Выберите сферу жизни и убедитесь, что в списке есть хотя бы два элемента.");
        return;
    }
    QPair<QString, QString> result = customDialogManager->showSortDialog(this);

    QString currentHeader = ui->mainHeader->text();

    QString sphereName = ui->sphereOfLife->currentItem()->data(Qt::UserRole).toString();

    ui->listGoalsAndAchieve->clear();


    // Выбор списка и сортировка в зависимости от заголовка
    if (currentHeader == "Цели") {
        sortGoals(result);
        showGoals(sphereName);
        updateDatabaseAfterSort("goals");
    } else if (currentHeader == "Достижения") {
        sortAchievements(result);
        showAchievements(sphereName);
        updateDatabaseAfterSort("achievements");
    } else if (currentHeader == "Запланировано") {
        sortPlans(result);
        showPlanned(sphereName);
        updateDatabaseAfterSort("planned_goals");
    }
    ui->listGoalsAndAchieve->update();
}

void EditorGoalsAndAchievement::sortGoals(const QPair<QString, QString>& result)
{
    if(result.first == "Имя"){
        sortListByName(goals, result.second);
    }
    else if(result.first == "Дата"){
        sortListByDate(goals, result.second);
    }
    else{
        sortListByPriority(goals, result.second);
    }
}

void EditorGoalsAndAchievement::sortAchievements(const QPair<QString, QString>& result)
{
    if(result.first == "Имя"){
        sortListByName(achievements, result.second);
    }
    else if(result.first == "Дата"){
        sortListByDate(achievements, result.second);
    }
    else{
        sortListByPriority(achievements, result.second);
    }
}

void EditorGoalsAndAchievement::sortPlans(const QPair<QString, QString>& result)
{
    if(result.first == "Имя"){
        sortListByName(plans, result.second);
    }
    else if(result.first == "Дата"){
        sortListByDate(plans, result.second);
    }
    else{
        sortListByPriority(plans, result.second);
    }
}

void EditorGoalsAndAchievement::sortListByName(QList<QPair<QString, QPair<QString, QPair<QString, int>>>>& list, const QString& order)
{
    std::sort(list.begin(), list.end(), [](const QPair<QString, QPair<QString, QPair<QString, int>>>& a, const QPair<QString, QPair<QString, QPair<QString, int>>>& b) {
        return a.first < b.first; // Сортировка по имени
    });
    if (order == "По убыванию") {
        std::reverse(list.begin(), list.end());
    }
}

void EditorGoalsAndAchievement::sortListByDate(QList<QPair<QString, QPair<QString, QPair<QString, int>>>>& list, const QString& order)
{
    std::sort(list.begin(), list.end(), [](const QPair<QString, QPair<QString, QPair<QString, int>>>& a, const QPair<QString, QPair<QString, QPair<QString, int>>>& b) {
        return QDate::fromString(a.second.first) < QDate::fromString(b.second.first); // Сортировка по дате
    });
    if (order == "По убыванию") {
        std::reverse(list.begin(), list.end());
    }
}

void EditorGoalsAndAchievement::sortListByPriority(QList<QPair<QString, QPair<QString, QPair<QString, int>>>>& list, const QString& order)
{
    std::sort(list.begin(), list.end(), [](const QPair<QString, QPair<QString, QPair<QString, int>>>& a, const QPair<QString, QPair<QString, QPair<QString, int>>>& b) {
        return a.second.second.second < b.second.second.second; // Сортировка по приоритету
    });
    if (order == "По убыванию") {
        std::reverse(list.begin(), list.end());
    }
}

void EditorGoalsAndAchievement::updateDatabaseAfterSort(const QString &table){
    QSqlQuery query;

    if (table == "goals") {
        for (int i = 0; i < goals.size(); ++i) {
            const auto& goal = goals[i];

            // Обновляем порядковый номер для каждой цели
            query.prepare("UPDATE goals SET date = :date, priority = :priority, "
                          "sphere_of_life_id = (SELECT id FROM sphere_of_life WHERE name = :sphere_name), "
                          "id = :id WHERE name = :name AND user_id = :user_id");
            query.bindValue(":date", goal.second.first);
            query.bindValue(":priority", goal.second.second.second);
            query.bindValue(":sphere_name", goal.second.second.first);
            query.bindValue(":name", goal.first);
            query.bindValue(":id", i); // Используем индекс для изменения порядка
            query.bindValue(":user_id", userId);
            if (!query.exec()) {
                qWarning() << "Error updating goal: " << query.lastError().text();
            }
        }
    }
    else if (table == "planned_goals") {
        // Важно: мы обновляем только те записи, которые изменили свою позицию
        for (int i = 0; i < plans.size(); ++i) {
            const auto& goal = plans[i];

            // Проверяем, изменился ли порядок записи
            query.prepare("UPDATE planned_goals SET planned_date = :planned_date, priority = :priority, "
                          "sphere_of_life_id = (SELECT id FROM sphere_of_life WHERE name = :sphere_name), "
                          "id = :id WHERE name = :name AND user_id = :user_id");
            query.bindValue(":planned_date", goal.second.first);
            query.bindValue(":priority", goal.second.second.second);
            query.bindValue(":sphere_name", goal.second.second.first);
            query.bindValue(":name", goal.first);
            query.bindValue(":id", i); // Используем индекс для изменения порядка
            query.bindValue(":user_id", userId);
            if (!query.exec()) {
                qWarning() << "Error updating goal: " << query.lastError().text();
            }
        }
    }
    else if (table == "achievements") {
        // Важно: мы обновляем только те записи, которые изменили свою позицию
        for (int i = 0; i < achievements.size(); ++i) {
            const auto& goal = achievements[i];

            // Проверяем, изменился ли порядок записи
            query.prepare("UPDATE achievements SET achievement_date = :achievement_date, priority = :priority, "
                          "sphere_of_life_id = (SELECT id FROM sphere_of_life WHERE name = :sphere_name), "
                          "id = :id WHERE name = :name AND user_id = :user_id");
            query.bindValue(":achievement_date", goal.second.first);
            query.bindValue(":priority", goal.second.second.second);
            query.bindValue(":sphere_name", goal.second.second.first);
            query.bindValue(":name", goal.first);
            query.bindValue(":id", i); // Используем индекс для изменения порядка
            query.bindValue(":user_id", userId);
            if (!query.exec()) {
                qWarning() << "Error updating goal: " << query.lastError().text();
            }
        }
    }
}

bool EditorGoalsAndAchievement::handleLogin(const QString &username, const QString &password) {
    // Проверяем подключение к базе данных
    if (!db.isOpen()) {
        qDebug() << "Database is not connected!";
        return false;
    }

    QSqlQuery query(db);

    // Проверяем, существует ли пользователь с таким логином
    query.prepare("SELECT id, password FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error executing query: " << query.lastError();
        return false;
    }

    if (query.next()) {
        userId = query.value("id").toInt();
        // Пользователь найден, проверяем пароль
        QString storedPassword = query.value("password").toString();
        QString enteredPasswordHash = hashPassword(password);

        if (storedPassword == enteredPasswordHash) {
            // Пароль совпал, успешный логин
            qDebug() << "Login successful!";
            return true;
        } else {
            // Неверный пароль
            QMessageBox::warning(this, "Ошибка", "Неверный пароль.");
            return false;
        }
    } else {
        QMessageBox::warning(this, "Ошибка", "Пользователь не найден.");
        return false;
    }
}

bool EditorGoalsAndAchievement::handleRegistration(const QString &username, const QString &password) {
    // Проверяем подключение к базе данных
    if (!db.isOpen()) {
        qDebug() << "Database is not connected!";
        return false;
    }

    QSqlQuery query(db);

    // Проверяем, существует ли пользователь с таким логином
    query.prepare("SELECT id FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error executing query: " << query.lastError();
        return false;
    }

    if (query.next()) {
        // Пользователь с таким логином уже существует
        QMessageBox::warning(this, "Ошибка", "Пользователь с таким логином уже существует.");
        return false;
    }

    // Регистрируем нового пользователя
    QString hashedPassword = hashPassword(password);

    query.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", hashedPassword);

    if (!query.exec()) {
        qDebug() << "Error executing query: " << query.lastError();
        return false;
    }

    userId = query.lastInsertId().toInt();

    // Успешная регистрация
    qDebug() << "Registration successful!";
    return true;
}


QString EditorGoalsAndAchievement::hashPassword(const QString &password) {
    // Хэшируем пароль с использованием SHA-256
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

void EditorGoalsAndAchievement::onEditGoalClicked(QListWidgetItem *item) {
    if (!item) return;

    QString name = item->data(Qt::UserRole).toString();
    QString date = item->data(Qt::UserRole + 1).toString();
    QString sphere = item->data(Qt::UserRole + 3).toString();
    int priority = item->data(Qt::UserRole + 2).toInt();

    QPair<QString, QPair<QString, QPair<QString, int>>> currentGoal = qMakePair(name, qMakePair(date, qMakePair(sphere, priority)));

    QPair<QString, QPair<QString, QPair<QString, int>>> result = customDialogManager->showEditGoal(this, ui->sphereOfLife, &currentGoal);

    if (!result.first.isEmpty()) {
        qDebug()<<"Result is not empty";
        for (auto &goal : goals) {
            qDebug()<<"Goal: "<<goal.first<<" "<<goal.second.first<<" "<<goal.second.second.first<<" "<<goal.second.second.second;
            qDebug()<<"Past: "<<name<<" "<<date<<" "<<sphere<<" "<<priority;
            if (goal.first == name && goal.second.first == date && goal.second.second.first == sphere && goal.second.second.second == priority) {
                goal.first = result.first;
                goal.second.first = result.second.first;
                goal.second.second.first = result.second.second.first;
                goal.second.second.second = result.second.second.second;
                qDebug()<<"New goal: "<<goal.first<<" "<<goal.second.first<<" "<<goal.second.second.first<<" "<<goal.second.second.second;
                break;
            }
        }

        if(ui->mainHeader->text() == "Цели"){
            QDate currentDate = QDate::currentDate();
            for (int i=0;i<goals.count();i++) {
                if (goals[i].first == name && goals[i].second.first == date && goals[i].second.second.first == sphere && goals[i].second.second.second == priority) {
                    if(currentDate < QDate::fromString(result.second.first)){
                        goals.removeAt(i);
                        plans.append(result);
                    }
                    else{
                        goals[i].first = result.first;
                        goals[i].second.first = result.second.first;
                        goals[i].second.second.first = result.second.second.first;
                        goals[i].second.second.second = result.second.second.second;
                    }
                    break;
                }
            }
            updateDatabaseAfterEdit("goals", result, name, sphere);
            showGoals(ui->sphereOfLife->currentItem()->data(Qt::UserRole).toString());
        }
        else if(ui->mainHeader->text() == "Достижения"){
            for (auto &achieve : achievements) {
                if (achieve.first == name && achieve.second.first == date && achieve.second.second.first == sphere && achieve.second.second.second == priority) {
                    achieve.first = result.first;
                    achieve.second.first = result.second.first;
                    achieve.second.second.first = result.second.second.first;
                    achieve.second.second.second = result.second.second.second;
                    break;
                }
            }
            updateDatabaseAfterEdit("achievements", result, name, sphere);
            showAchievements(ui->sphereOfLife->currentItem()->data(Qt::UserRole).toString());
        }
        else{
            QDate currentDate = QDate::currentDate();
            for (int i=0;i<plans.count();i++) {
                if (plans[i].first == name && plans[i].second.first == date && plans[i].second.second.first == sphere && plans[i].second.second.second == priority) {
                    if(currentDate >= QDate::fromString(result.second.first)){
                        plans.removeAt(i);
                        goals.append(result);
                    }
                    else{
                        plans[i].first = result.first;
                        plans[i].second.first = result.second.first;
                        plans[i].second.second.first = result.second.second.first;
                        plans[i].second.second.second = result.second.second.second;
                    }
                    break;
                }
            }
            updateDatabaseAfterEdit("planned_goals", result, name, sphere);
            showPlanned(ui->sphereOfLife->currentItem()->data(Qt::UserRole).toString());
        }

        if (result.second.second.first != sphere) {
            updateSphereCount(sphere, -1);
            updateSphereCount(result.second.second.first, 1);
        }

        updateProgressBar(ui->sphereOfLife->currentItem(), nullptr);
    }


}

void EditorGoalsAndAchievement::updateDatabaseAfterEdit(const QString &tableName,
                                                        const QPair<QString, QPair<QString, QPair<QString, int>>> &newGoal,
                                                        const QString &oldName,
                                                        const QString &oldSphere) {
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QMap<QString, QString> dateFieldMap;
    dateFieldMap["goals"] = "date";
    dateFieldMap["achievements"] = "achievement_date";
    dateFieldMap["planned_goals"] = "planned_date";

    QString dateField = dateFieldMap.value(tableName, "date");

    int oldSphereID=0;
    int newSphereID=0;

    // Update the goal in the database
    QSqlQuery query;
    query.prepare("SELECT id FROM sphere_of_life WHERE name = :name");
    query.bindValue(":name", oldSphere);
    if (query.exec() && query.next()) {
        oldSphereID = query.value(0).toInt();
    }

    query.prepare("SELECT id FROM sphere_of_life WHERE name = :name");
    query.bindValue(":name", newGoal.second.second.first);
    if (query.exec() && query.next()) {
        newSphereID = query.value(0).toInt();
    }

    query.prepare(QString("UPDATE %1 SET name = :name, %2 = :date, sphere_of_life_id = :sphereId, priority = :priority "
                          "WHERE name = :oldName AND sphere_of_life_id = :oldSphereId AND user_id = :user_id")
                      .arg(tableName).arg(dateField));
    query.bindValue(":name", newGoal.first);
    query.bindValue(":date", newGoal.second.first);
    query.bindValue(":sphereId", newSphereID);
    query.bindValue(":priority", newGoal.second.second.second);
    query.bindValue(":oldName", oldName);
    query.bindValue(":oldSphereId", oldSphereID);
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Error updating goal in database:" << query.lastError().text();
        db.rollback();
        return;
    }

    // If sphere has changed, update sphere counts
    // if (newGoal.second.second.first != oldSphere) {
    //     // Decrease count in old sphere
    //     query.prepare("UPDATE sphere_of_life SET count = count - 1 WHERE name = :oldSphere AND user_id = :user_id");
    //     query.bindValue(":oldSphere", oldSphere);
    //     query.bindValue(":user_id", userId);
    //     if (!query.exec()) {
    //         qDebug() << "Error updating old sphere count:" << query.lastError().text();
    //         db.rollback();
    //         return;
    //     }

    //     // Increase count in new sphere
    //     query.prepare("UPDATE sphere_of_life SET count = count + 1 WHERE name = :newSphere AND user_id = :user_id");
    //     query.bindValue(":newSphere", newGoal.second.second.first);
    //     query.bindValue(":user_id", userId);
    //     if (!query.exec()) {
    //         qDebug() << "Error updating new sphere count:" << query.lastError().text();
    //         db.rollback();
    //         return;
    //     }
    // }

    db.commit();
}
