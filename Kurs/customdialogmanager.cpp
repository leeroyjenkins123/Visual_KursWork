#include "customdialogmanager.h"

CustomDialogManager::CustomDialogManager(QObject *parent)
    : QObject{parent}
{}

QString CustomDialogManager::showAddSphereOfLife(QWidget *parent){
    QDialog dialog(parent);
    dialog.setFixedSize(400,200);
    dialog.setWindowTitle("Добавить сферу жизни");
    dialog.setStyleSheet("background-color: #5860EE; color: white; font: 14pt 'Ubuntu Sans';");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLineEdit *lineEdit = new QLineEdit(&dialog);
    lineEdit->setPlaceholderText("Введите название сферы жизни");
    QPushButton *doneButton = new QPushButton("Добавить", &dialog);
    QPushButton *cancelButton = new QPushButton("Отменить", &dialog);

    layout->addWidget(lineEdit);
    layout->addWidget(doneButton);
    layout->addWidget(cancelButton);

    QString result;

    connect(doneButton, &QPushButton::clicked, [&dialog, &result, lineEdit](){
        QString input = lineEdit->text();
        if (input.isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Поле не может быть пустым!", QMessageBox::Ok);
        } else if (input.length() > 20) {
            QMessageBox::warning(&dialog, "Ошибка", "Сфера жизни не может превышать 20 символов!", QMessageBox::Ok);
        } else {
            result = input;
            dialog.accept();
        }
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    dialog.exec();
    return result;
}

QPair<QString, QPair<QString, QPair<QString, int>>> CustomDialogManager::showAddGoal(QWidget *parent, QListWidget *sphereOfLifeList){
    QDialog dialog(parent);
    dialog.setFixedSize(400,200);
    dialog.setWindowTitle("Добавить цель");
    dialog.setStyleSheet("background-color: #5860EE; color: white; font: 14pt 'Ubuntu Sans';");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *nameLabel = new QLabel("Название цели:", &dialog);
    QLineEdit *goalNameEdit = new QLineEdit(&dialog);
    goalNameEdit->setPlaceholderText("Введите название цели");

    QLabel *sphereLabel = new QLabel("Сфера жизни:", &dialog);
    QComboBox *sphereComboBox = new QComboBox(&dialog);

    // Add items from QListWidget to QComboBox
    if(sphereOfLifeList->count()>0){
        for (int i = 0; i < sphereOfLifeList->count(); ++i) {
            QListWidgetItem *item = sphereOfLifeList->item(i);
            if (item) {
                sphereComboBox->addItem(item->data(Qt::UserRole).toString());
            }
        }
    }

    QLabel *dateLabel = new QLabel("Дата выполнения:", &dialog);
    QDateEdit *dateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    dateEdit->setCalendarPopup(true);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("Добавить", &dialog);
    QPushButton *cancelButton = new QPushButton("Отменить", &dialog);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(cancelButton);

    QLabel *priorityLabel = new QLabel("Приоритет задачи (1-100):", &dialog);
    QSlider *prioritySlider = new QSlider(Qt::Horizontal, &dialog);
    prioritySlider->setRange(1, 100);
    prioritySlider->setValue(50); // Default value

    QLabel *priorityValueLabel = new QLabel("50", &dialog);
    priorityValueLabel->setAlignment(Qt::AlignCenter);

    connect(prioritySlider, &QSlider::valueChanged, [&priorityValueLabel](int value) {
        priorityValueLabel->setText(QString::number(value));
    });

    QHBoxLayout *priorityLayout = new QHBoxLayout();
    priorityLayout->addWidget(priorityLabel);
    priorityLayout->addWidget(prioritySlider);
    priorityLayout->addWidget(priorityValueLabel);

    layout->addWidget(nameLabel);
    layout->addWidget(goalNameEdit);
    layout->addWidget(sphereLabel);
    layout->addWidget(sphereComboBox);
    layout->addWidget(dateLabel);
    layout->addWidget(dateEdit);
    layout->addLayout(priorityLayout);
    layout->addLayout(buttonLayout);

    QPair<QString, QPair<QString, QPair<QString, int>>> result;

    connect(addButton, &QPushButton::clicked, [&dialog, &result, goalNameEdit, sphereComboBox, dateEdit, prioritySlider]() {
        QString goalName = goalNameEdit->text();
        if (goalName.isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Поле 'Название цели' не может быть пустым!", QMessageBox::Ok);
        } else if (goalName.length() > 20) {
            QMessageBox::warning(&dialog, "Ошибка", "Название цели не может превышать 20 символов!", QMessageBox::Ok);
        } else {
            result.first = goalName;
            result.second.second.first = sphereComboBox->currentText();
            result.second.first = dateEdit->date().toString("dd.MM.yyyy");
            result.second.second.second = prioritySlider->value();
            dialog.accept();
        }
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
    return result;
}

QPair<QString, QPair<QString, QPair<QString, int>>> CustomDialogManager::showEditGoal(QWidget *parent, QListWidget *sphereOfLifeList, const QPair<QString, QPair<QString, QPair<QString, int>>>* currentGoal){
    QDialog dialog(parent);
    dialog.setFixedSize(400,200);
    dialog.setWindowTitle("Редактировать цель");
    dialog.setStyleSheet("background-color: #5860EE; color: white; font: 14pt 'Ubuntu Sans';");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *nameLabel = new QLabel("Название цели:", &dialog);
    QLineEdit *goalNameEdit = new QLineEdit(&dialog);
    goalNameEdit->setPlaceholderText("Введите название цели");

    QLabel *sphereLabel = new QLabel("Сфера жизни:", &dialog);
    QComboBox *sphereComboBox = new QComboBox(&dialog);

    // Add items from QListWidget to QComboBox
    if(sphereOfLifeList->count()>0){
        for (int i = 0; i < sphereOfLifeList->count(); ++i) {
            QListWidgetItem *item = sphereOfLifeList->item(i);
            if (item) {
                sphereComboBox->addItem(item->data(Qt::UserRole).toString());
            }
        }
    }

    QLabel *dateLabel = new QLabel("Дата выполнения:", &dialog);
    QDateEdit *dateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    dateEdit->setCalendarPopup(true);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("Добавить", &dialog);
    QPushButton *cancelButton = new QPushButton("Отменить", &dialog);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(cancelButton);

    QLabel *priorityLabel = new QLabel("Приоритет задачи (1-100):", &dialog);
    QSlider *prioritySlider = new QSlider(Qt::Horizontal, &dialog);
    prioritySlider->setRange(1, 100);
    prioritySlider->setValue(50); // Default value

    QLabel *priorityValueLabel = new QLabel("50", &dialog);
    priorityValueLabel->setAlignment(Qt::AlignCenter);

    connect(prioritySlider, &QSlider::valueChanged, [&priorityValueLabel](int value) {
        priorityValueLabel->setText(QString::number(value));
    });

    QHBoxLayout *priorityLayout = new QHBoxLayout();
    priorityLayout->addWidget(priorityLabel);
    priorityLayout->addWidget(prioritySlider);
    priorityLayout->addWidget(priorityValueLabel);

    layout->addWidget(nameLabel);
    layout->addWidget(goalNameEdit);
    layout->addWidget(sphereLabel);
    layout->addWidget(sphereComboBox);
    layout->addWidget(dateLabel);
    layout->addWidget(dateEdit);
    layout->addLayout(priorityLayout);
    layout->addLayout(buttonLayout);

    if (currentGoal) {
        goalNameEdit->setText(currentGoal->first);
        sphereComboBox->setCurrentText(currentGoal->second.second.first);
        dateEdit->setDate(QDate::fromString(currentGoal->second.first, "dd.MM.yyyy"));
        prioritySlider->setValue(currentGoal->second.second.second);
    }

    QPair<QString, QPair<QString, QPair<QString, int>>> result;

    connect(addButton, &QPushButton::clicked, [&dialog, &result, goalNameEdit, sphereComboBox, dateEdit, prioritySlider]() {
        QString goalName = goalNameEdit->text();
        if (goalName.isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Поле 'Название цели' не может быть пустым!", QMessageBox::Ok);
        } else if (goalName.length() > 20) {
            QMessageBox::warning(&dialog, "Ошибка", "Название цели не может превышать 20 символов!", QMessageBox::Ok);
        } else {
            result.first = goalName;
            result.second.second.first = sphereComboBox->currentText();
            result.second.first = dateEdit->date().toString("dd.MM.yyyy");
            result.second.second.second = prioritySlider->value();
            dialog.accept();
        }
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
    return result;
}

QPair<QString, QString> CustomDialogManager::showSortDialog(QWidget *parent) {
    QDialog dialog(parent);
    dialog.setFixedSize(400, 200);
    dialog.setWindowTitle("Настройки сортировки");
    dialog.setStyleSheet("background-color: #5860EE; color: white; font: 14pt 'Ubuntu Sans';");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Поле для выбора критерия сортировки (Имя, Дата, Приоритет)
    QLabel *sortByLabel = new QLabel("Сортировать по:", &dialog);
    QComboBox *sortByComboBox = new QComboBox(&dialog);
    sortByComboBox->addItem("Имя");
    sortByComboBox->addItem("Дата");
    sortByComboBox->addItem("Приоритет");

    // Поле для выбора порядка сортировки (По возрастанию / По убыванию)
    QLabel *orderLabel = new QLabel("Порядок сортировки:", &dialog);
    QComboBox *orderComboBox = new QComboBox(&dialog);
    orderComboBox->addItem("По возрастанию");
    orderComboBox->addItem("По убыванию");

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *applyButton = new QPushButton("Применить", &dialog);
    QPushButton *cancelButton = new QPushButton("Отменить", &dialog);
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);

    layout->addWidget(sortByLabel);
    layout->addWidget(sortByComboBox);
    layout->addWidget(orderLabel);
    layout->addWidget(orderComboBox);
    layout->addLayout(buttonLayout);

    // Результат
    QPair<QString, QString> result;

    connect(applyButton, &QPushButton::clicked, [&dialog, &result, sortByComboBox, orderComboBox]() {
        result.first = sortByComboBox->currentText();  // Критерий сортировки
        result.second = orderComboBox->currentText();  // Порядок сортировки
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
    return result;
}

QPair<QPair<QString, QString>, bool> CustomDialogManager::showLoginRegisterDialog(QWidget *parent) {
    QDialog dialog(parent);
    dialog.setFixedSize(400, 300);
    dialog.setWindowTitle("Вход / Регистрация");
    dialog.setStyleSheet("background-color: #5860EE; color: white; font: 14pt 'Ubuntu Sans';");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // Создаем QTabWidget
    QTabWidget *tabWidget = new QTabWidget(&dialog);
    mainLayout->addWidget(tabWidget);

    // Вкладка "Вход"
    QWidget *loginPage = new QWidget(&dialog);
    QVBoxLayout *loginLayout = new QVBoxLayout(loginPage);

    QLabel *loginLabel = new QLabel("Логин:", loginPage);
    QLineEdit *loginEdit = new QLineEdit(loginPage);
    loginEdit->setPlaceholderText("Введите логин");

    QLabel *passwordLabel = new QLabel("Пароль:", loginPage);
    QLineEdit *passwordEdit = new QLineEdit(loginPage);
    passwordEdit->setPlaceholderText("Введите пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);

    QPushButton *loginButton = new QPushButton("Войти", loginPage);

    loginLayout->addWidget(loginLabel);
    loginLayout->addWidget(loginEdit);
    loginLayout->addWidget(passwordLabel);
    loginLayout->addWidget(passwordEdit);
    loginLayout->addWidget(loginButton);

    loginPage->setLayout(loginLayout);

    // Вкладка "Регистрация"
    QWidget *registerPage = new QWidget(&dialog);
    QVBoxLayout *registerLayout = new QVBoxLayout(registerPage);

    QLabel *registerLoginLabel = new QLabel("Логин:", registerPage);
    QLineEdit *registerLoginEdit = new QLineEdit(registerPage);
    registerLoginEdit->setPlaceholderText("Введите логин");

    QLabel *registerPasswordLabel = new QLabel("Пароль:", registerPage);
    QLineEdit *registerPasswordEdit = new QLineEdit(registerPage);
    registerPasswordEdit->setPlaceholderText("Введите пароль");
    registerPasswordEdit->setEchoMode(QLineEdit::Password);

    QLabel *confirmPasswordLabel = new QLabel("Подтвердите пароль:", registerPage);
    QLineEdit *confirmPasswordEdit = new QLineEdit(registerPage);
    confirmPasswordEdit->setPlaceholderText("Подтвердите пароль");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    QPushButton *registerButton = new QPushButton("Зарегистрироваться", registerPage);

    registerLayout->addWidget(registerLoginLabel);
    registerLayout->addWidget(registerLoginEdit);
    registerLayout->addWidget(registerPasswordLabel);
    registerLayout->addWidget(registerPasswordEdit);
    registerLayout->addWidget(confirmPasswordLabel);
    registerLayout->addWidget(confirmPasswordEdit);
    registerLayout->addWidget(registerButton);

    registerPage->setLayout(registerLayout);

    // Добавляем вкладки в QTabWidget
    tabWidget->addTab(loginPage, "Вход");
    tabWidget->addTab(registerPage, "Регистрация");

    // Структура для хранения логина, пароля и флага
    QPair<QPair<QString, QString>, bool> result;
    bool isLogin = true;  // По умолчанию вход

    QRegularExpression allowedCharsRegex("^[a-zA-Z0-9!@#$%^&*()-_=+\\[\\]{};:'\"<>,.?/]+$");

    auto isInputValid = [&](const QString& input) {
        return allowedCharsRegex.match(input).hasMatch();
    };

    // Логика кнопки "Войти"
    connect(loginButton, &QPushButton::clicked, [&]() {
        QString login = loginEdit->text();
        QString password = passwordEdit->text();

        if (login.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Пожалуйста, заполните все поля.");
            return;
        }

        if (!isInputValid(login)) {
            QMessageBox::warning(&dialog, "Ошибка", "Логин содержит недопустимые символы.");
            return;
        }

        if (!isInputValid(password)) {
            QMessageBox::warning(&dialog, "Ошибка", "Пароль содержит недопустимые символы.");
            return;
        }

        result.first = QPair<QString, QString>(login, password);
        dialog.accept();  // Закрыть диалог
    });

    // Логика кнопки "Зарегистрироваться"
    connect(registerButton, &QPushButton::clicked, [&]() {
        QString login = registerLoginEdit->text();
        QString password = registerPasswordEdit->text();
        QString confirmPassword = confirmPasswordEdit->text();

        if (login.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Пожалуйста, заполните все поля.");
            return;
        }

        if (login.length() < 10 || login.length() > 30) {
            QMessageBox::warning(&dialog, "Ошибка", "Логин должен быть от 10 до 30 символов.");
            return;
        }

        if (password.length() < 10 || password.length() > 30) {
            QMessageBox::warning(&dialog, "Ошибка", "Пароль должен быть от 10 до 30 символов.");
            return;
        }

        if (!isInputValid(login)) {
            QMessageBox::warning(&dialog, "Ошибка", "Логин содержит недопустимые символы.");
            return;
        }

        if (!isInputValid(password)) {
            QMessageBox::warning(&dialog, "Ошибка", "Пароль содержит недопустимые символы.");
            return;
        }

        if (password != confirmPassword) {
            QMessageBox::warning(&dialog, "Ошибка", "Пароли не совпадают.");
            return;
        }

        result.first = QPair<QString, QString>(login, password);
        dialog.accept();  // Закрыть диалог
    });

    // Переключаем флаг, если была выбрана регистрация
    connect(tabWidget, &QTabWidget::currentChanged, [&]() {
        isLogin = tabWidget->currentIndex() == 0;  // Если активна вкладка "Вход", флаг true, если "Регистрация" — false
    });

    dialog.exec();  // Запуск диалога

    result.second = isLogin;  // Устанавливаем флаг: вход или регистрация
    return result;
}
