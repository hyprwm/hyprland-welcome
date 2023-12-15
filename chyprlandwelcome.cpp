#include "chyprlandwelcome.h"
#include "./ui_chyprlandwelcome.h"
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QTabBar>
#include <QTimer>
#include <QProcess>
#include <filesystem>

QWidget* findByName(QString name) {
    QList<QWidget*>           widgets = QApplication::allWidgets();
    QList<QWidget*>::iterator it      = std::find_if(widgets.begin(), widgets.end(), [name](QWidget* widget) -> bool { return widget->objectName() == name; });
    return it == widgets.end() ? nullptr : *it;
}

std::string execAndGet(std::string cmd) {
    cmd += " 2>&1";
    std::array<char, 128>                          buffer;
    std::string                                    result;
    const std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        return "";

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

bool appExists(std::string binName) {
    std::string ret = execAndGet("which " + binName);
    return ret.find("not found") == std::string::npos && ret.find("no " + binName + " in") == std::string::npos && ret.find("/") != std::string::npos;
}

bool binExists(std::string path) {
    std::string ret = execAndGet("[ -f " + path + " ] || echo \"no\"");
    return ret.find("no") == std::string::npos;
}

bool appIsRunning(std::string binName) {
    std::string ret = execAndGet("pidof " + binName);
    return !ret.empty();
}

struct SAppCheck {
    std::vector<std::string> binaryNames;
    std::vector<std::string> binaryPaths;
    std::string              name;
    QLabel*                  label    = nullptr;
    bool                     needsAllInstalled = false;
    bool                     needsAllRunning = false;
    std::vector<std::string> runningNames;
};

std::vector<SAppCheck> appChecks;

//
void CHyprlandWelcome::startAppTimer() {

    appChecks.push_back(SAppCheck{{"waybar", "ags", "eww"}, {}, "Status Bar", (QLabel*)findByName("INSTALL_BAR"), false, false, {"waybar", "gjs", "eww-daemon", "eww"}});
    appChecks.push_back(SAppCheck{{"dolphin", "thunar", "ranger"}, {}, "File Manager", (QLabel*)findByName("INSTALL_FM")});
    appChecks.push_back(SAppCheck{{"mako", "dunst"}, {}, "Notification Daemon", (QLabel*)findByName("INSTALL_NOTIF")});
    appChecks.push_back(SAppCheck{{"anyrun", "fuzzel", "wofi", "tofi"}, {}, "App Launcher", (QLabel*)findByName("INSTALL_LAUNCHER")});
    appChecks.push_back(SAppCheck{{"hyprpaper", "swaybg", "swww"}, {}, "Wallpaper Utility", (QLabel*)findByName("INSTALL_WALLPAPER")});
    appChecks.push_back(SAppCheck{{"pipewire", "wireplumber"}, {}, "Pipewire*", (QLabel*)findByName("INSTALL_PW"), true, true});
    appChecks.push_back(SAppCheck{{"xdg-desktop-portal", "xdg-desktop-portal-hyprland", "xdg-desktop-portal-gtk"},
                                  {"/usr/lib/xdg-desktop-portal", "/usr/lib/xdg-desktop-portal-hyprland", "/usr/lib/xdg-desktop-portal-gtk", "/usr/libexec/xdg-desktop-portal",
                                   "/usr/libexec/xdg-desktop-portal-hyprland", "/usr/libexec/xdg-desktop-portal-gtk"},
                                  "XDG Desktop Portal*",
                                  (QLabel*)findByName("INSTALL_XDP"), false, true, {"xdg-desktop-portal", "xdg-desktop-portal-hyprland"}});
    appChecks.push_back(SAppCheck{{}, {"/usr/lib/polkit-kde-authentication-agent-1"}, "Authentication Agent", (QLabel*)findByName("INSTALL_AUTH"), false, false, {"polkit-kde-authentication-agent-1"}});
    appChecks.push_back(SAppCheck{{"qtwaylandscanner"}, {}, "QT Wayland Support", (QLabel*)findByName("INSTALL_QTW")});
    appChecks.push_back(SAppCheck{{"kitty", "wezterm", "alacritty", "foot", "konsole", "gnome-terminal"}, {}, "Terminal", (QLabel*)findByName("INSTALL_TERM")});
    appChecks.push_back(SAppCheck{{"qt5ct", "qt6ct"}, {}, "QT Theming", (QLabel*)findByName("INSTALL_QTTHEME")});

    QTimer* timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, this, [this]() {
        if (this->currentTab != 1)
            return;

        for (const auto& app : appChecks) {
            std::vector<std::string> found;
            std::vector<std::string> runningApps;
            bool running = false;
            for (const auto& bin : app.binaryNames) {
                if (appExists(bin))
                    found.push_back(bin);

                if (app.runningNames.empty() && appIsRunning(bin)) {
                    runningApps.push_back(bin);
                    running = true;
                }
            }
            if (!app.runningNames.empty()) for (const auto& bin : app.runningNames) {
                if (appIsRunning(bin)) {
                    runningApps.push_back(bin);
                    running = true;
                }
            }

            for (const auto& bin : app.binaryPaths) {
                if (binExists(bin))
                    found.push_back(bin);
            }

            if ((found.empty() && !running) || (app.needsAllInstalled && found.size() != app.binaryNames.size() + app.binaryPaths.size())) {
                app.label->setText(QString::fromStdString("<html><head/><body><p><span style=\" color:#ff0000;\">❌</span> " + app.name + "</p></body></html>"));
            } else if (app.needsAllRunning && runningApps.size() != (app.runningNames.empty() ? app.binaryNames : app.runningNames).size()) {
                app.label->setText(QString::fromStdString("<html><head/><body><p><span style=\" color:#ff0000;\">❌</span> " + app.name + ": Found all, but not running.</p></body></html>"));
            } else if (!running) {
                std::string text = "<html><head/><body><p><span style=\" color:#00ff00;\">✔️</span> " + app.name + ": <span style=\" color:#00ff00;\">found</span> ";
                for (auto& f : found)
                    text += f + ", ";
                text.pop_back();
                text.pop_back();
                text += "</p></body></html>";
                app.label->setText(QString::fromStdString(text));
            } else {
                std::string text = "<html><head/><body><p><span style=\" color:#00ffff;\">✔️</span> " + app.name + ": <span style=\" color:#00ffff;\">running</span> ";
                for (auto& f : runningApps)
                    text += f + ", ";
                text.pop_back();
                text.pop_back();
                text += "</p></body></html>";
                app.label->setText(QString::fromStdString(text));
            }
        }
    });
    timer->start(1000);
}

std::string getDataStatePath() {
    const auto HOME = getenv("HOME");
    if (!HOME)
        return "";

    const auto XDG_DATA_HOME = getenv("XDG_DATA_HOME");

    if (XDG_DATA_HOME)
        return std::string{XDG_DATA_HOME} + "/hyprland-welcome";
    return std::string{HOME} + "/.local/share/hyprland-welcome";
}

void CHyprlandWelcome::exitDontShowAgain() {
    const auto STATEPATH = getDataStatePath();
    if (STATEPATH.empty()) {
        QApplication::exit(0);
        return;
    }

    const auto CONFPATH = STATEPATH + "/state";

    if (!std::filesystem::exists(STATEPATH))
        std::filesystem::create_directories(STATEPATH);

    execAndGet("echo \"noshow\" > " + CONFPATH);
    QApplication::exit(0);
}

CHyprlandWelcome::CHyprlandWelcome(QWidget* parent) : QMainWindow(parent), ui(new Ui::CHyprlandWelcome) {

    ui->setupUi(this);

    const auto GHBUTTON = (QPushButton*)findByName("githubButton");
    QObject::connect(GHBUTTON, &QPushButton::clicked, [] { QDesktopServices::openUrl(QString{"https://github.com/hyprwm/hyprland"}); });
    const auto WIKIBUTTON = (QPushButton*)findByName("openWikiButton");
    QObject::connect(WIKIBUTTON, &QPushButton::clicked, [] { QDesktopServices::openUrl(QString{"https://wiki.hyprland.org/Configuring/Configuring-Hyprland/"}); });

    const auto EXITBUTTON = (QPushButton*)findByName("exitButton");
    QObject::connect(EXITBUTTON, &QPushButton::clicked, [] { QApplication::exit(0); });

    const auto TABS = (QTabWidget*)findByName("tabs");
    QObject::connect(TABS->tabBar(), &QTabBar::currentChanged, [TABS, this] { TABS->tabBar()->setCurrentIndex(this->currentTab); });

    const auto TAB1NEXT = (QPushButton*)findByName("tab1Next");
    QObject::connect(TAB1NEXT, &QPushButton::clicked, [TABS, this] {
        this->currentTab = 1;
        TABS->tabBar()->setCurrentIndex(this->currentTab);
    });
    const auto TAB2NEXT = (QPushButton*)findByName("tab2Next");
    QObject::connect(TAB2NEXT, &QPushButton::clicked, [TABS, this] {
        this->currentTab = 2;
        TABS->tabBar()->setCurrentIndex(this->currentTab);
    });
    const auto TAB3NEXT = (QPushButton*)findByName("tab3Next");
    QObject::connect(TAB3NEXT, &QPushButton::clicked, [TABS, this] {
        this->currentTab = 3;
        TABS->tabBar()->setCurrentIndex(this->currentTab);
    });
    const auto TAB2BACK = (QPushButton*)findByName("tab2Back");
    QObject::connect(TAB2BACK, &QPushButton::clicked, [TABS, this] {
        this->currentTab = 0;
        TABS->tabBar()->setCurrentIndex(this->currentTab);
    });
    const auto TAB3BACK = (QPushButton*)findByName("tab3Back");
    QObject::connect(TAB3BACK, &QPushButton::clicked, [TABS, this] {
        this->currentTab = 1;
        TABS->tabBar()->setCurrentIndex(this->currentTab);
    });
    const auto TAB4BACK = (QPushButton*)findByName("tab4Back");
    QObject::connect(TAB4BACK, &QPushButton::clicked, [TABS, this] {
        this->currentTab = 2;
        TABS->tabBar()->setCurrentIndex(this->currentTab);
    });

    const auto OPENTERMINAL = (QPushButton*)findByName("openTerminalButton");
    QObject::connect(OPENTERMINAL, &QPushButton::clicked, [TABS, this] {
        QProcess* term = new QProcess(this);
        term->start("/bin/sh", QStringList{"-c", "kitty || alacritty || wezterm || foot || konsole || gnome-terminal || xterm"});
    });
    const auto OPENTERMINAL2 = (QPushButton*)findByName("openTerminalButton2");
    QObject::connect(OPENTERMINAL2, &QPushButton::clicked, [TABS, this] {
        QProcess* term = new QProcess(this);
        term->start("/bin/sh", QStringList{"-c", "kitty || alacritty || wezterm || foot || konsole || gnome-terminal || xterm"});
    });

    const auto EXITDONTSHOW = (QPushButton*)findByName("dontShowAgain");
    const auto FINISH       = (QPushButton*)findByName("finishButton");
    QObject::connect(EXITDONTSHOW, &QPushButton::clicked, [this] { this->exitDontShowAgain(); });
    QObject::connect(FINISH, &QPushButton::clicked, [this] { this->exitDontShowAgain(); });

    TABS->tabBar()->setCurrentIndex(this->currentTab);

    startAppTimer();
}

CHyprlandWelcome::~CHyprlandWelcome() {
    delete ui;
}
