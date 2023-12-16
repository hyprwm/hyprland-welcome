#ifndef CHYPRLANDWELCOME_H
#define CHYPRLANDWELCOME_H

#include <QMainWindow>
#include <mutex>

QT_BEGIN_NAMESPACE
namespace Ui {
    class CHyprlandWelcome;
}
QT_END_NAMESPACE

class CHyprlandWelcome : public QMainWindow {
    Q_OBJECT

  public:
    CHyprlandWelcome(QWidget* parent = nullptr);
    ~CHyprlandWelcome();

  private:
    Ui::CHyprlandWelcome* ui;

    void                  startAppTimer();
    void                  exitDontShowAgain();

    int                   currentTab = 0;
    bool                  exit       = false;
    std::mutex            appScanMutex;
};
#endif // CHYPRLANDWELCOME_H
