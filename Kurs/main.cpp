#include "editorgoalsandachievement.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EditorGoalsAndAchievement w;
    if (w.isVisible()) {
        // Если диалог не закрылся из-за отмены входа/регистрации, показываем главное окно
        w.show();
    } else {
        // Если диалог закрылся с отменой входа/регистрации, завершаем приложение
        return 0;
    }
    return a.exec();
}
