#ifndef CHYPRLANDWELCOME_H
#define CHYPRLANDWELCOME_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class CHyprlandWelcome; }
QT_END_NAMESPACE

class CHyprlandWelcome : public QMainWindow
{
    Q_OBJECT

public:
    CHyprlandWelcome(QWidget *parent = nullptr);
    ~CHyprlandWelcome();

private:
    Ui::CHyprlandWelcome *ui;

    void startAppTimer();
    void exitDontShowAgain();

    int currentTab = 0;
};
#endif // CHYPRLANDWELCOME_H
