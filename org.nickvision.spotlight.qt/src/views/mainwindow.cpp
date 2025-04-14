#include "views/mainwindow.h"
#include <cmath>
#include <format>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <libnick/filesystem/userdirectories.h>
#include <libnick/helpers/codehelpers.h>
#include <libnick/localization/gettext.h>
#include <libnick/notifications/shellnotification.h>
#include <oclero/qlementine/widgets/LoadingSpinner.hpp>
#include "controls/aboutdialog.h"
#include "controls/infobar.h"
#include "helpers/qthelpers.h"
#include "views/settingsdialog.h"

#define RESIZE_TIMER_TIMEOUT 100
#define IMAGE_PREFERED_WIDTH 400
#define IMAGES_PER_ROW static_cast<int>(std::floor(m_ui->viewStack->geometry().width() / static_cast<double>(IMAGE_PREFERED_WIDTH)))
#define IMAGE_SPACING 4
#define IMAGE_WIDTH (IMAGE_PREFERED_WIDTH - (IMAGES_PER_ROW * IMAGE_SPACING))
#define IMAGE_HEIGHT 220

using namespace Nickvision::App;
using namespace Nickvision::Events;
using namespace Nickvision::Filesystem;
using namespace Nickvision::Helpers;
using namespace Nickvision::Notifications;
using namespace Nickvision::Spotlight::Qt::Controls;
using namespace Nickvision::Spotlight::Qt::Helpers;
using namespace Nickvision::Spotlight::Shared::Controllers;
using namespace Nickvision::Spotlight::Shared::Models;
using namespace Nickvision::Update;
using namespace oclero::qlementine;

enum MainWindowPage
{
    Loading = 0,
    Images
};

namespace Ui
{
    class MainWindow
    {
    public:
        void setupUi(Nickvision::Spotlight::Qt::Views::MainWindow* parent)
        {
            viewStack = new QStackedWidget(parent);
            //Actions
            actionExportAll = new QAction(parent);
            actionExportAll->setText(_("Export All"));
            actionExportAll->setIcon(QLEMENTINE_ICON(Action_SaveToDisk));
            actionExportAll->setShortcut(QKeyCombination(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_S));
            actionExit = new QAction(parent);
            actionExit->setText(_("Exit"));
            actionExit->setIcon(QLEMENTINE_ICON(Action_Close));
            actionExit->setShortcut(Qt::CTRL | Qt::Key_Q);
            actionClearAndSync = new QAction(parent);
            actionClearAndSync->setText(_("Clear and Sync"));
            actionClearAndSync->setIcon(QLEMENTINE_ICON(Action_Refresh));
            actionClearAndSync->setShortcut(Qt::CTRL | Qt::Key_R);
            actionSettings = new QAction(parent);
            actionSettings->setText(_("Settings"));
            actionSettings->setIcon(QLEMENTINE_ICON(Navigation_Settings));
            actionSettings->setShortcut(Qt::CTRL | Qt::Key_Comma);
            actionExport = new QAction(parent);
            actionExport->setText(_("Export"));
            actionExport->setIcon(QLEMENTINE_ICON(Action_Save));
            actionExport->setShortcut(Qt::CTRL | Qt::Key_S);
            actionSetAsBackground = new QAction(parent);
            actionSetAsBackground->setText(_("Set as Background"));
            actionSetAsBackground->setIcon(QLEMENTINE_ICON(Software_Desktop));
            actionSetAsBackground->setShortcut(Qt::CTRL | Qt::Key_B);
            actionCheckForUpdates = new QAction(parent);
            actionCheckForUpdates->setText(_("Check for Updates"));
            actionCheckForUpdates->setIcon(QLEMENTINE_ICON(Action_Update));
            actionGitHubRepo = new QAction(parent);
            actionGitHubRepo->setText(_("GitHub Repo"));
            actionGitHubRepo->setIcon(QLEMENTINE_ICON(Software_VersionControl));
            actionReportABug = new QAction(parent);
            actionReportABug->setText(_("Report a Bug"));
            actionReportABug->setIcon(QLEMENTINE_ICON(Misc_Debug));
            actionDiscussions = new QAction(parent);
            actionDiscussions->setText(_("Discussions"));
            actionDiscussions->setIcon(QLEMENTINE_ICON(Misc_Users));
            actionAbout = new QAction(parent);
            actionAbout->setText(_("About Spotlight"));
            actionAbout->setIcon(QLEMENTINE_ICON(Misc_Info));
            actionAbout->setShortcut(Qt::Key_F1);
            //InfoBar
            infoBar = new Nickvision::Spotlight::Qt::Controls::InfoBar(parent);
            parent->addDockWidget(::Qt::BottomDockWidgetArea, infoBar);
            //MenuBar
            QMenu* menuFile{ new QMenu(parent) };
            menuFile->setTitle(_("File"));
            menuFile->addAction(actionExportAll);
            menuFile->addSeparator();
            menuFile->addAction(actionExit);
            QMenu* menuEdit{ new QMenu(parent) };
            menuEdit->setTitle(_("Edit"));
            menuEdit->addAction(actionClearAndSync);
            menuEdit->addSeparator();
            menuEdit->addAction(actionSettings);
            QMenu* menuImage{ new QMenu(parent) };
            menuImage->setTitle(_("Image"));
            menuImage->addAction(actionExport);
            menuImage->addAction(actionSetAsBackground);
            QMenu* menuHelp{ new QMenu(parent) };
            menuHelp->setTitle(_("Help"));
            menuHelp->addAction(actionCheckForUpdates);
            menuHelp->addSeparator();
            menuHelp->addAction(actionGitHubRepo);
            menuHelp->addAction(actionReportABug);
            menuHelp->addAction(actionDiscussions);
            menuHelp->addSeparator();
            menuHelp->addAction(actionAbout);
            parent->menuBar()->addMenu(menuFile);
            parent->menuBar()->addMenu(menuEdit);
            parent->menuBar()->addMenu(menuImage);
            parent->menuBar()->addMenu(menuHelp);
            //StatusBar
            lblStatus = new QLabel(parent);
            parent->statusBar()->addWidget(lblStatus);
            //ToolBar
            QToolBar* toolBar{ new QToolBar(parent) };
            toolBar->setAllowedAreas(::Qt::ToolBarArea::TopToolBarArea);
            toolBar->setMovable(false);
            toolBar->setFloatable(false);
            toolBar->addAction(actionExport);
            toolBar->addAction(actionExportAll);
            toolBar->addSeparator();
            toolBar->addAction(actionSetAsBackground);
            parent->addToolBar(toolBar);
            //Loading Page
            LoadingSpinner* spinner{ new LoadingSpinner(parent) };
            spinner->setMinimumSize(32, 32);
            spinner->setMaximumSize(32, 32);
            spinner->setSpinning(true);
            QVBoxLayout* layoutSpinner{ new QVBoxLayout() };
            layoutSpinner->addStretch();
            layoutSpinner->addWidget(spinner);
            layoutSpinner->addStretch();
            QHBoxLayout* layoutLoading{ new QHBoxLayout() };
            layoutLoading->addStretch();
            layoutLoading->addLayout(layoutSpinner);
            layoutLoading->addStretch();
            QWidget* loadingPage{ new QWidget(parent) };
            loadingPage->setLayout(layoutLoading);
            viewStack->addWidget(loadingPage);
            //Images Page
            grdImages = new QGridLayout();
            QVBoxLayout* layoutImages{ new QVBoxLayout() };
            layoutImages->addLayout(grdImages);
            layoutImages->addStretch();
            QWidget* wdgImages{ new QWidget(parent) };
            wdgImages->setLayout(layoutImages);
            QScrollArea* scrollImages{ new QScrollArea(parent) };
            scrollImages->setWidgetResizable(true);
            scrollImages->setVerticalScrollBarPolicy(::Qt::ScrollBarPolicy::ScrollBarAsNeeded);
            scrollImages->setHorizontalScrollBarPolicy(::Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
            scrollImages->setWidget(wdgImages);
            viewStack->addWidget(scrollImages);
            //Main Layout
            parent->setCentralWidget(viewStack);
        }

        QAction* actionExportAll;
        QAction* actionExit;
        QAction* actionClearAndSync;
        QAction* actionSettings;
        QAction* actionExport;
        QAction* actionSetAsBackground;
        QAction* actionCheckForUpdates;
        QAction* actionGitHubRepo;
        QAction* actionReportABug;
        QAction* actionDiscussions;
        QAction* actionAbout;
        Nickvision::Spotlight::Qt::Controls::InfoBar* infoBar;
        QLabel* lblStatus;
        QStackedWidget* viewStack;
        QGridLayout* grdImages;
    };
}

namespace Nickvision::Spotlight::Qt::Views
{
    MainWindow::MainWindow(const std::shared_ptr<MainWindowController>& controller, oclero::qlementine::ThemeManager* themeManager, QWidget* parent) 
        : QMainWindow{ parent },
        m_ui{ new Ui::MainWindow() },
        m_controller{ controller },
        m_themeManager{ themeManager },
        m_resizeTimer{ new QTimer(this) },
        m_selectedImage{ nullptr }
    {
        //Window Settings
        bool stable{ m_controller->getAppInfo().getVersion().getVersionType() == VersionType::Stable };
        setWindowTitle(stable ? _("Spotlight") : _("Spotlight (Preview)"));
        setWindowIcon(QIcon(":/icon.ico"));
        //Load Ui
        m_ui->setupUi(this);
        //Signals
        connect(m_ui->actionExportAll, &QAction::triggered, this, &MainWindow::exportAllImages);
        connect(m_ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
        connect(m_ui->actionClearAndSync, &QAction::triggered, this, &MainWindow::clearAndSync);
        connect(m_ui->actionSettings, &QAction::triggered, this, &MainWindow::settings);
        connect(m_ui->actionExport, &QAction::triggered, this, &MainWindow::exportImage);
        connect(m_ui->actionSetAsBackground, &QAction::triggered, this, &MainWindow::setImageAsBackground);
        connect(m_ui->actionCheckForUpdates, &QAction::triggered, this, &MainWindow::checkForUpdates);
        connect(m_ui->actionGitHubRepo, &QAction::triggered, this, &MainWindow::gitHubRepo);
        connect(m_ui->actionReportABug, &QAction::triggered, this, &MainWindow::reportABug);
        connect(m_ui->actionDiscussions, &QAction::triggered, this, &MainWindow::discussions);
        connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
        connect(m_resizeTimer, &QTimer::timeout, this, &MainWindow::onWindowResize);
        m_controller->notificationSent() += [&](const NotificationSentEventArgs& args) { QtHelpers::dispatchToMainThread([this, args]() { onNotificationSent(args); }); };
        m_controller->imagesSynced() += [&](const ParamEventArgs<std::vector<std::filesystem::path>>& args) { QtHelpers::dispatchToMainThread([this, args]() { onImagesSynced(args); }); };
    }

    MainWindow::~MainWindow()
    {
        delete m_ui;
    }

    void MainWindow::show()
    {
        QMainWindow::show();
        const StartupInformation& info{ m_controller->startup(reinterpret_cast<HWND>(winId())) };
        setGeometry(QWidget::geometry().x(), QWidget::geometry().y(), info.getWindowGeometry().getWidth(), info.getWindowGeometry().getHeight());
        if(info.getWindowGeometry().isMaximized())
        {
            showMaximized();
        }
        m_ui->viewStack->setCurrentIndex(MainWindowPage::Loading);
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        if(!m_controller->canShutdown())
        {
            return event->ignore();
        }
        m_controller->shutdown({ geometry().width(), geometry().height(), isMaximized() });
        event->accept();
    }

    void MainWindow::resizeEvent(QResizeEvent* event)
    {
        m_resizeTimer->stop();
        m_resizeTimer->start(RESIZE_TIMER_TIMEOUT);
    }

    void MainWindow::exportAllImages()
    {
        if(m_controller->getSpotlightImageCount() == 0)
        {
            return;
        }
        QString folder{ QFileDialog::getExistingDirectory(this, _("Export All Images"), QString::fromStdString(UserDirectories::get(UserDirectory::Pictures).string())) };
        if(!folder.isEmpty())
        {
            m_controller->exportAllImages(folder.toStdString());
        }
    }

    void MainWindow::exit()
    {
        close();
    }

    void MainWindow::clearAndSync()
    {
        QMessageBox message{ QMessageBox::Icon::Question, _("Clear and Sync?"), _("Are you sure you want to clear the image cache and re-sync the spotlight images?"), QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, this };
        message.setDefaultButton(QMessageBox::StandardButton::No);
        if(message.exec() == QMessageBox::StandardButton::Yes)
        {
            m_ui->viewStack->setCurrentIndex(MainWindowPage::Loading);
            m_controller->clearAndSync();
        }
    }

    void MainWindow::settings()
    {
        SettingsDialog dialog{ m_controller->createPreferencesViewController(), m_themeManager, this };
        dialog.exec();
    }

    void MainWindow::exportImage()
    {
        if(!m_selectedImage)
        {
            return;
        }
        QString file{ QFileDialog::getSaveFileName(this, _("Export Image"), QString::fromStdString(UserDirectories::get(UserDirectory::Pictures).string()), "JPEG (*.jpg)") };
        if(!file.isEmpty())
        {
            m_controller->exportImage(m_selectedImage->index(), file.toStdString());
        }
    }

    void MainWindow::setImageAsBackground()
    {
        if(!m_selectedImage)
        {
            return;
        }
        m_controller->setImageAsDesktopBackground(m_selectedImage->index());
    }

    void MainWindow::checkForUpdates()
    {
        m_controller->checkForUpdates();
    }

    void MainWindow::windowsUpdate()
    {
        m_controller->windowsUpdate();
    }

    void MainWindow::gitHubRepo()
    {
        QDesktopServices::openUrl(QString::fromStdString(m_controller->getAppInfo().getSourceRepo()));
    }

    void MainWindow::reportABug()
    {
        QDesktopServices::openUrl(QString::fromStdString(m_controller->getAppInfo().getIssueTracker()));
    }

    void MainWindow::discussions()
    {
        QDesktopServices::openUrl(QString::fromStdString(m_controller->getAppInfo().getSupportUrl()));
    }

    void MainWindow::about()
    {
        AboutDialog dialog{ m_controller->getAppInfo(), m_controller->getDebugInformation(), this };
        dialog.exec();
    }

    void MainWindow::onWindowResize()
    {
        m_resizeTimer->stop();
        int row{ 0 };
        int col{ 0 };
        for(SpotlightImage* image : m_images)
        {
            m_ui->grdImages->removeWidget(image);
        }
        for(SpotlightImage* image : m_images)
        {
            image->resize(IMAGE_WIDTH, IMAGE_HEIGHT);
            m_ui->grdImages->addWidget(image, row, col);
            col++;
            if(col == IMAGES_PER_ROW)
            {
                row++;
                col = 0;
            }
        }
    }

    void MainWindow::onNotificationSent(const NotificationSentEventArgs& args)
    {
        QString actionText;
        std::function<void()> actionCallback;
        if(args.getAction() == "update")
        {
            actionText = _("Update");
            actionCallback = [this]() { windowsUpdate(); };
        }
        m_ui->infoBar->show(args, actionText, actionCallback);
    }

    void MainWindow::onImagesSynced(const ParamEventArgs<std::vector<std::filesystem::path>>& args)
    {
        if((*args).empty())
        {
            QMessageBox::critical(this, _("No Spotlight Images Found"), _("Ensure Windows Spotlight is enabled and come back later to try again. The application will now close."), QMessageBox::StandardButton::Close);
            close();
            return;
        }
        int row{ 0 };
        int col{ 0 };
        for(SpotlightImage* image : m_images)
        {
            m_ui->grdImages->removeWidget(image);
            delete image;
        }
        m_images.clear();
        m_selectedImage = nullptr;
        m_ui->viewStack->setCurrentIndex(MainWindowPage::Images);
        m_ui->lblStatus->setText(QString::fromStdString(std::vformat(_("Total Number of Images: {}"), std::make_format_args(CodeHelpers::unmove((*args).size())))));
        for(const std::filesystem::path& image : args.getParam())
        {
            SpotlightImage* img{ new SpotlightImage(col + (row * IMAGES_PER_ROW), image, IMAGE_WIDTH, IMAGE_HEIGHT, this) };
            connect(img, &SpotlightImage::clicked, [this, img]()
            {
                if(m_selectedImage)
                {
                    m_selectedImage->setSelected(false);
                }
                img->setSelected(true);
                m_selectedImage = img;
            });
            m_ui->grdImages->addWidget(img, row, col);
            m_images.push_back(img);
            col++;
            if(col == IMAGES_PER_ROW)
            {
                row++;
                col = 0;
            }
        }
    }
}
