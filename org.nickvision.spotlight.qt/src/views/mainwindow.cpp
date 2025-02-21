#include "views/mainwindow.h"
#include <cmath>
#include <format>
#include <QDesktopServices>
#include <QFileDialog>
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
#include "controls/aboutdialog.h"
#include "controls/infobar.h"
#include "helpers/qthelpers.h"
#include "views/settingsdialog.h"

using namespace Nickvision::App;
using namespace Nickvision::Events;
using namespace Nickvision::Filesystem;
using namespace Nickvision::Helpers;
using namespace Nickvision::Notifications;
using namespace Nickvision::Spotlight::Qt::Controls;
using namespace Nickvision::Spotlight::Qt::Helpers;
using namespace Nickvision::Spotlight::Shared::Controllers;
using namespace Nickvision::Spotlight::Shared::Events;
using namespace Nickvision::Spotlight::Shared::Models;
using namespace Nickvision::Update;

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
            actionExportAll->setShortcut(Qt::CTRL | Qt::SHIFT || Qt::Key_S);
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
            actionGrid = new QAction(parent);
            actionGrid->setText(_("Grid"));
            actionGrid->setCheckable(true);
            actionGrid->setIcon(QLEMENTINE_ICON(Misc_ItemsGrid));
            actionFlip = new QAction(parent);
            actionFlip->setText(_("Flip"));
            actionFlip->setCheckable(true);
            actionFlip->setIcon(QLEMENTINE_ICON(Navigation_ArrowRight));
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
            QMenu* menuView{ new QMenu(parent) };
            QMenu* menuMode{ new QMenu(parent) };
            menuView->setTitle(_("View"));
            menuMode->setTitle(_("Mode"));
            menuMode->addAction(actionGrid);
            menuMode->addAction(actionFlip);
            menuView->addMenu(menuMode);
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
            parent->menuBar()->addMenu(menuView);
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
            QWidget* pageLoading{ new QWidget(parent) };
            QHBoxLayout* layoutLoading{ new QHBoxLayout(parent) };
            QProgressBar* progressBar{ new QProgressBar(parent) };
            progressBar->setRange(0, 0);
            progressBar->setValue(-1);
            progressBar->setTextVisible(false);
            progressBar->setInvertedAppearance(false);
            progressBar->setAlignment(::Qt::AlignCenter);
            layoutLoading->addWidget(progressBar);
            pageLoading->setLayout(layoutLoading);
            viewStack->addWidget(pageLoading);
            //Grid Page

            //Flip Page

            //Main Layout
            QWidget* centralWidget{ new QWidget(parent) };
            QVBoxLayout* layoutMain{ new QVBoxLayout(parent) };
            layoutMain->setContentsMargins(6, 6, 6, 6);
            layoutMain->addWidget(viewStack);
            centralWidget->setLayout(layoutMain);
            parent->setCentralWidget(centralWidget);
        }

        QAction* actionExportAll;
        QAction* actionExit;
        QAction* actionClearAndSync;
        QAction* actionSettings;
        QAction* actionGrid;
        QAction* actionFlip;
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
    };
}

namespace Nickvision::Spotlight::Qt::Views
{
    enum MainWindowPage
    {
        Loading = 0,
        Grid,
        Flip,
    };

    MainWindow::MainWindow(const std::shared_ptr<MainWindowController>& controller, oclero::qlementine::ThemeManager* themeManager, QWidget* parent) 
        : QMainWindow{ parent },
        m_ui{ new Ui::MainWindow() },
        m_controller{ controller },
        m_themeManager{ themeManager },
        m_resizeTimer{ this }
    {
        //Window Settings
        bool stable{ m_controller->getAppInfo().getVersion().getVersionType() == VersionType::Stable };
        setWindowTitle(stable ? _("Application") : _("Application (Preview)"));
        setWindowIcon(QIcon(":/icon.svg"));
        setAcceptDrops(true);
        //Load Ui
        m_ui->setupUi(this);
        //Signals
        connect(m_ui->actionExportAll, &QAction::triggered, this, &MainWindow::exportAllImages);
        connect(m_ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
        connect(m_ui->actionClearAndSync, &QAction::triggered, this, &MainWindow::clearAndSync);
        connect(m_ui->actionSettings, &QAction::triggered, this, &MainWindow::settings);
        connect(m_ui->actionGrid, &QAction::toggled, this, &MainWindow::gridMode);
        connect(m_ui->actionFlip, &QAction::toggled, this, &MainWindow::flipMode);
        connect(m_ui->actionExport, &QAction::triggered, this, &MainWindow::exportImage);
        connect(m_ui->actionSetAsBackground, &QAction::triggered, this, &MainWindow::setImageAsBackground);
        connect(m_ui->actionCheckForUpdates, &QAction::triggered, this, &MainWindow::checkForUpdates);
        connect(m_ui->actionGitHubRepo, &QAction::triggered, this, &MainWindow::gitHubRepo);
        connect(m_ui->actionReportABug, &QAction::triggered, this, &MainWindow::reportABug);
        connect(m_ui->actionDiscussions, &QAction::triggered, this, &MainWindow::discussions);
        connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
        connect(m_ui->tblImages, &QTableWidget::cellClicked, this, &MainWindow::onTblImagesSelectionChanged);
        connect(m_ui->tblImages, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTblImagesDoubleClicked);
        connect(m_ui->sliderFlip, &QSlider::valueChanged, this, &MainWindow::onSliderFlipChanged);
        connect(m_ui->btnFlipNext, &QPushButton::clicked, this, &MainWindow::flipNext);
        connect(m_ui->btnFlipPrev, &QPushButton::clicked, this, &MainWindow::flipPrev);
        m_controller->notificationSent() += [&](const NotificationSentEventArgs& args) { QtHelpers::dispatchToMainThread([this, args]() { onNotificationSent(args); }); };
        m_controller->shellNotificationSent() += [&](const ShellNotificationSentEventArgs& args) { onShellNotificationSent(args); };
        m_controller->imagesSynced() += [&](const ImagesSyncedEventArgs& args) { QtHelpers::dispatchToMainThread([this, args]() { onImagesSynced(args); }); };
    }

    MainWindow::~MainWindow()
    {
        delete m_infoBar;
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
        m_controller->shutdown({ geometry().width(), geometry().height(), isMaximized() }, m_ui->actionGrid->isChecked() ? ViewMode::Grid : ViewMode::Flip);
        event->accept();
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

    void MainWindow::gridMode(bool toggled)
    {
        if(toggled)
        {
            m_ui->viewStack->setCurrentIndex(MainWindowPage::Grid);
            m_ui->actionFlip->setChecked(false);
        }
    }

    void MainWindow::flipMode(bool toggled)
    {
        if(toggled)
        {
            m_ui->viewStack->setCurrentIndex(MainWindowPage::Flip);
            if(m_ui->actionGrid->isChecked())
            {
                onSliderFlipChanged(m_ui->sliderFlip->value());    
            }
            m_ui->actionGrid->setChecked(false);
        }
    }

    void MainWindow::exportImage()
    {
        if(!m_ui->tblImages->selectionModel()->hasSelection())
        {
            return;
        }
        if(m_ui->actionGrid->isChecked())
        {
            QString file{ QFileDialog::getSaveFileName(this, _("Export Image"), QString::fromStdString(UserDirectories::get(UserDirectory::Pictures).string()), "JPEG (*.jpg)") };
            if(!file.isEmpty())
            {
                m_controller->exportImage(m_ui->tblImages->selectionModel()->currentIndex().row() * m_ui->tblImages->columnCount() + m_ui->tblImages->selectionModel()->currentIndex().column(), file.toStdString());
            }
        }
        else
        {
            QString file{ QFileDialog::getSaveFileName(this, _("Export Image"), QString::fromStdString(UserDirectories::get(UserDirectory::Pictures).string()), "JPEG (*.jpg)") };
            if(!file.isEmpty())
            {
                m_controller->exportImage(m_ui->sliderFlip->value(), file.toStdString());
            }
        }
    }

    void MainWindow::setImageAsBackground()
    {
        if(!m_ui->tblImages->selectionModel()->hasSelection())
        {
            return;
        }
        if(m_ui->actionGrid->isChecked())
        {
            m_controller->setImageAsDesktopBackground(m_ui->tblImages->selectionModel()->currentIndex().row() * m_ui->tblImages->columnCount() + m_ui->tblImages->selectionModel()->currentIndex().column());
        }
        else
        {
            m_controller->setImageAsDesktopBackground(m_ui->sliderFlip->value());
        }
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

    void MainWindow::loadGridView(const std::vector<std::filesystem::path>& images)
    {
        int columnCount{ static_cast<int>(std::floor(m_ui->tblImages->width() / static_cast<double>(m_ui->tblImages->horizontalHeader()->defaultSectionSize()))) };
        int rowCount{ static_cast<int>(std::ceil(images.size() / static_cast<double>(columnCount))) };
        if(columnCount == m_ui->tblImages->columnCount())
        {
            return;
        }
        m_ui->tblImages->clear();
        m_ui->tblImages->setColumnCount(columnCount);
        m_ui->tblImages->setRowCount(rowCount);
        for(int i = 0; i < rowCount; i++)
        {
            for(int j = 0; j < columnCount; j++)
            {
                int index{ i * columnCount + j };
                if(index >= images.size())
                {
                    break;
                }
                QPixmap pixmap{ QString::fromStdString(images[index].string()) };
                QLabel* lbl{ new QLabel() };
                lbl->setScaledContents(true);
                lbl->setPixmap(pixmap.scaled(m_ui->tblImages->horizontalHeader()->defaultSectionSize(), m_ui->tblImages->verticalHeader()->defaultSectionSize(), ::Qt::KeepAspectRatio, ::Qt::FastTransformation));
                m_ui->tblImages->setCellWidget(i, j, lbl);
                if(i == 0 && j == 0)
                {
                    lbl->setFrameStyle(QFrame::Box);
                    lbl->setLineWidth(3);
                }
                qApp->processEvents();
            }
        }
    }

    void MainWindow::onTblImagesSelectionChanged(int row, int column)
    {
        static QLabel* lastSelected{ static_cast<QLabel*>(m_ui->tblImages->cellWidget(0, 0)) };
        lastSelected->setFrameStyle(QFrame::NoFrame);
        lastSelected = static_cast<QLabel*>(m_ui->tblImages->cellWidget(row, column));
        lastSelected->setFrameStyle(QFrame::Box);
        lastSelected->setLineWidth(3);
    }

    void MainWindow::onTblImagesDoubleClicked(int row, int column)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(m_controller->getSpotlightImagePath(row * m_ui->tblImages->columnCount() + column).string())));
    }

    void MainWindow::onSliderFlipChanged(int value)
    {
        const std::filesystem::path& image{ m_controller->getSpotlightImagePath(value) };
        QPixmap pixmap{ QString::fromStdString(image.string()) };
        m_ui->lblFlipImage->setPixmap(pixmap.scaled(m_ui->scrollFlip->width() - 20, m_ui->scrollFlip->height() - 20, ::Qt::KeepAspectRatio));
    }

    void MainWindow::flipNext()
    {
        if(m_ui->sliderFlip->value() == m_ui->sliderFlip->maximum())
        {
            m_ui->sliderFlip->setValue(0);
        }
        else
        {
            m_ui->sliderFlip->setValue(m_ui->sliderFlip->value() + 1);
        }
    }

    void MainWindow::flipPrev()
    {
        if(m_ui->sliderFlip->value() == 0)
        {
            m_ui->sliderFlip->setValue(m_ui->sliderFlip->maximum());
        }
        else
        {
            m_ui->sliderFlip->setValue(m_ui->sliderFlip->value() - 1);
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

    void MainWindow::onShellNotificationSent(const ShellNotificationSentEventArgs& args)
    {
        ShellNotification::send(args, reinterpret_cast<HWND>(winId()));
    }

    void MainWindow::onImagesSynced(const ImagesSyncedEventArgs& args)
    {
        if(args.getImages().empty())
        {
            QMessageBox::critical(this, _("No Spotlight Images Found"), _("Ensure Windows Spotlight is enabled and come back later to try again. The application will now close."), QMessageBox::StandardButton::Close);
            close();
            return;
        }
        m_lblStatus->setText(QString::fromStdString(std::vformat(_("Total Number of Images: {}"), std::make_format_args(CodeHelpers::unmove(args.getImages().size())))));
        //Setup Flip Page (Faster)
        m_ui->sliderFlip->setMaximum(args.getImages().size() - 1);
        m_ui->sliderFlip->setValue(0);
        onSliderFlipChanged(0);
        //Setup Grid Page
        loadGridView(args.getImages());
        m_ui->actionGrid->setChecked(false);
        m_ui->actionFlip->setChecked(false);
        //Change Page
        if(args.getViewMode() == ViewMode::Grid)
        {
            m_ui->actionGrid->setChecked(true);
        }
        else
        {
            m_ui->actionFlip->setChecked(true);
        }
    }
}