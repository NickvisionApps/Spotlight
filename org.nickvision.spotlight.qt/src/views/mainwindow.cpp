#include "views/mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <format>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <libnick/filesystem/userdirectories.h>
#include <libnick/helpers/codehelpers.h>
#include <libnick/localization/gettext.h>
#include <libnick/notifications/shellnotification.h>
#include "controls/aboutdialog.h"
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

namespace Nickvision::Spotlight::Qt::Views
{
    enum MainWindowPage
    {
        Grid = 0,
        Flip,
        Loading
    };

    MainWindow::MainWindow(const std::shared_ptr<MainWindowController>& controller, oclero::qlementine::ThemeManager* themeManager, QWidget* parent) 
        : QMainWindow{ parent },
        m_ui{ new Ui::MainWindow() },
        m_infoBar{ new InfoBar(this) },
        m_controller{ controller },
        m_themeManager{ themeManager },
        m_resizeTimer{ this },
        m_lblStatus{ nullptr }
    {
        m_ui->setupUi(this);
        setWindowTitle(m_controller->getAppInfo().getVersion().getVersionType() == VersionType::Stable ? _("Spotlight") : _("Spotlight (Preview)"));
        addDockWidget(::Qt::BottomDockWidgetArea, m_infoBar);
        //MenuBar
        m_ui->menuFile->setTitle(_("File"));
        m_ui->actionExport->setText(_("Export"));
        m_ui->actionExportAll->setText(_("Export All"));
        m_ui->actionExit->setText(_("Exit"));
        m_ui->menuEdit->setTitle(_("Edit"));
        m_ui->actionClearAndSync->setText(_("Clear and Sync"));
        m_ui->actionSettings->setText(_("Settings"));
        m_ui->menuView->setTitle(_("View"));
        m_ui->menuMode->setTitle(_("Mode"));
        m_ui->actionGrid->setText(_("Grid"));
        m_ui->actionFlip->setText(_("Flip"));
        m_ui->menuImage->setTitle(_("Image"));
        m_ui->actionSetAsBackground->setText(_("Set as Background"));
        m_ui->menuHelp->setTitle(_("Help"));
        m_ui->actionCheckForUpdates->setText(_("Check for Updates"));
        m_ui->actionGitHubRepo->setText(_("GitHub Repo"));
        m_ui->actionReportABug->setText(_("Report a Bug"));
        m_ui->actionDiscussions->setText(_("Discussions"));
        m_ui->actionAbout->setText(_("About Spotlight"));
        //StatusBar
        m_lblStatus = new QLabel(this);
        m_ui->statusBar->addWidget(m_lblStatus);
        //Grid Page
        m_resizeTimer.setSingleShot(true);
        m_resizeTimer.setInterval(300);
        //Flip Page
        m_ui->btnFlipPrev->setToolTip(_("Previous"));
        m_ui->btnFlipNext->setToolTip(_("Next"));
        //Signals
        connect(m_ui->actionExport, &QAction::triggered, this, &MainWindow::exportImage);
        connect(m_ui->actionExportAll, &QAction::triggered, this, &MainWindow::exportAllImages);
        connect(m_ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
        connect(m_ui->actionClearAndSync, &QAction::triggered, this, &MainWindow::clearAndSync);
        connect(m_ui->actionSettings, &QAction::triggered, this, &MainWindow::settings);
        connect(m_ui->actionGrid, &QAction::toggled, this, &MainWindow::gridMode);
        connect(m_ui->actionFlip, &QAction::toggled, this, &MainWindow::flipMode);
        connect(m_ui->actionSetAsBackground, &QAction::triggered, this, &MainWindow::setImageAsBackground);
        connect(m_ui->actionCheckForUpdates, &QAction::triggered, this, &MainWindow::checkForUpdates);
        connect(m_ui->actionGitHubRepo, &QAction::triggered, this, &MainWindow::gitHubRepo);
        connect(m_ui->actionReportABug, &QAction::triggered, this, &MainWindow::reportABug);
        connect(m_ui->actionDiscussions, &QAction::triggered, this, &MainWindow::discussions);
        connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
        //connect(&m_resizeTimer, &QTimer::timeout, this, &MainWindow::loadGridView);
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

    void MainWindow::resizeEvent(QResizeEvent* event)
    {
        QMainWindow::resizeEvent(event);
        if(m_ui->actionGrid->isChecked())
        {
            m_resizeTimer.start();
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
        m_infoBar->show(args, actionText, actionCallback);
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