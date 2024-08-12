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
using namespace Nickvision::Spotlight::QT::Controls;
using namespace Nickvision::Spotlight::QT::Helpers;
using namespace Nickvision::Spotlight::Shared::Controllers;
using namespace Nickvision::Spotlight::Shared::Models;
using namespace Nickvision::Update;

namespace Nickvision::Spotlight::QT::Views
{
    MainWindow::MainWindow(const std::shared_ptr<MainWindowController>& controller, QWidget* parent) 
        : QMainWindow{ parent },
        m_ui{ new Ui::MainWindow() },
        m_controller{ controller }
    {
        m_ui->setupUi(this);
        setWindowTitle(m_controller->getAppInfo().getVersion().getVersionType() == VersionType::Stable ? _("Spotlight") : _("Spotlight (Preview)"));
        //Localize Menu Strings
        m_ui->menuFile->setTitle(_("File"));
        m_ui->actionExport->setText(_("Export"));
        m_ui->actionExportAll->setText(_("Export All"));
        m_ui->actionExit->setText(_("Exit"));
        m_ui->menuEdit->setTitle(_("Edit"));
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
        //Localize Grid Page
        //Localize Flip Page
        m_ui->btnFlipPrev->setToolTip(_("Previous"));
        m_ui->btnFlipNext->setToolTip(_("Next"));
        //Signals
        connect(m_ui->actionExport, &QAction::triggered, this, &MainWindow::exportImage);
        connect(m_ui->actionExportAll, &QAction::triggered, this, &MainWindow::exportAllImages);
        connect(m_ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
        connect(m_ui->actionSettings, &QAction::triggered, this, &MainWindow::settings);
        connect(m_ui->actionGrid, &QAction::toggled, this, &MainWindow::gridMode);
        connect(m_ui->actionFlip, &QAction::toggled, this, &MainWindow::flipMode);
        connect(m_ui->actionSetAsBackground, &QAction::triggered, this, &MainWindow::setImageAsBackground);
        connect(m_ui->actionCheckForUpdates, &QAction::triggered, this, &MainWindow::checkForUpdates);
        connect(m_ui->actionGitHubRepo, &QAction::triggered, this, &MainWindow::gitHubRepo);
        connect(m_ui->actionReportABug, &QAction::triggered, this, &MainWindow::reportABug);
        connect(m_ui->actionDiscussions, &QAction::triggered, this, &MainWindow::discussions);
        connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
        connect(m_ui->sliderFlip, &QSlider::valueChanged, this, &MainWindow::onSliderFlipChanged);
        connect(m_ui->btnFlipNext, &QPushButton::clicked, this, &MainWindow::flipNext);
        connect(m_ui->btnFlipPrev, &QPushButton::clicked, this, &MainWindow::flipPrev);
        m_controller->notificationSent() += [&](const NotificationSentEventArgs& args) { QTHelpers::dispatchToMainThread([this, args]() { onNotificationSent(args); }); };
        m_controller->shellNotificationSent() += [&](const ShellNotificationSentEventArgs& args) { onShellNotificationSent(args); };
        m_controller->imagesSynced() += [&](const EventArgs& args) { QTHelpers::dispatchToMainThread([this]() { onImagesSynced(); }); };
    }

    MainWindow::~MainWindow()
    {
        delete m_ui;
    }

    void MainWindow::show()
    {
        QMainWindow::show();
#ifdef _WIN32
        WindowGeometry geometry{ m_controller->startup(reinterpret_cast<HWND>(winId())) };
#elif defined(__linux__)
        WindowGeometry geometry{ m_controller->startup(m_controller->getAppInfo().getId() + ".desktop") };
#else
        WindowGeometry geometry{ m_controller->startup() };
#endif
        if(geometry.isMaximized())
        {
            showMaximized();
        }
        else
        {
            setGeometry(QWidget::geometry().x(), QWidget::geometry().y(), geometry.getWidth(), geometry.getHeight());
        }
        if(m_controller->getViewMode() == ViewMode::Grid)
        {
            m_ui->actionGrid->setChecked(true);
        }
        else
        {
            m_ui->actionFlip->setChecked(true);
        }
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

    void MainWindow::exportImage()
    {
        if(m_controller->getSpotlightImages().empty())
        {
            return;
        }
        if(m_ui->actionGrid->isChecked())
        {

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
        if(m_controller->getSpotlightImages().empty())
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

    void MainWindow::settings()
    {
        SettingsDialog dialog{ m_controller->createPreferencesViewController(), this };
        dialog.exec();
    }

    void MainWindow::gridMode(bool toggled)
    {
        if(toggled)
        {
            m_ui->viewStack->setCurrentIndex(0);
            m_ui->actionFlip->setChecked(false);
        }
    }

    void MainWindow::flipMode(bool toggled)
    {
        if(toggled)
        {
            m_ui->viewStack->setCurrentIndex(1);
            if(m_ui->actionGrid->isChecked())
            {
                onSliderFlipChanged(m_ui->sliderFlip->value());    
            }
            m_ui->actionGrid->setChecked(false);
        }
    }

    void MainWindow::setImageAsBackground()
    {
        if(m_controller->getSpotlightImages().empty())
        {
            return;
        }
        if(m_ui->actionGrid->isChecked())
        {

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

#ifdef _WIN32
    void MainWindow::windowsUpdate()
    {
        m_controller->windowsUpdate();
    }
#endif

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

    void MainWindow::onSliderFlipChanged(int value)
    {
        const std::filesystem::path image{ m_controller->getSpotlightImages()[value] };
        QPixmap pixmap{ QString::fromStdString(image.string()) };
        m_ui->lblFlipImage->setPixmap(pixmap.scaled(m_ui->scrollFlip->width() - 20, m_ui->scrollFlip->height() - 20, Qt::KeepAspectRatio, Qt::FastTransformation));
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
        QMessageBox::Icon icon{ QMessageBox::Icon::NoIcon };
        switch(args.getSeverity())
        {
        case NotificationSeverity::Informational:
        case NotificationSeverity::Success:
            icon = QMessageBox::Icon::Information;
            break;
        case NotificationSeverity::Warning:
            icon = QMessageBox::Icon::Warning;
            break;
        case NotificationSeverity::Error:
            icon = QMessageBox::Icon::Critical;
            break;
        }
        QMessageBox msgBox{ icon, QString::fromStdString(m_controller->getAppInfo().getShortName()), QString::fromStdString(args.getMessage()), QMessageBox::StandardButton::Ok, this };
        if(args.getAction() == "update")
        {
            QPushButton* updateButton{ msgBox.addButton(_("Update"), QMessageBox::ButtonRole::ActionRole) };
            connect(updateButton, &QPushButton::clicked, this, &MainWindow::checkForUpdates);
        }
        msgBox.exec();
    }

    void MainWindow::onShellNotificationSent(const ShellNotificationSentEventArgs& args)
    {
        m_controller->log(Logging::LogLevel::Info, "ShellNotification sent. (" + args.getMessage() + ")");
#ifdef _WIN32
        ShellNotification::send(args, reinterpret_cast<HWND>(winId()));
#elif defined(__linux__)
        ShellNotification::send(args, m_controller->getAppInfo().getId(), _("Open"));
#endif
    }

    void MainWindow::onImagesSynced()
    {
        if(m_controller->getSpotlightImages().empty())
        {
            QMessageBox::critical(this, _("No Spotlight Images Found"), _("Ensure Windows Spotlight is enabled and come back later to try again. The application will now close."), QMessageBox::StandardButton::Close);
            close();
            return;
        }
        QLabel* lblStatus{ new QLabel() };
        lblStatus->setText(QString::fromStdString(std::vformat(_("Total Number of Images: {}"), std::make_format_args(CodeHelpers::unmove(m_controller->getSpotlightImages().size())))));
        m_ui->statusBar->addWidget(lblStatus);
        //Setup Grid Page
        int rowCount{ static_cast<int>(std::ceil(m_controller->getSpotlightImages().size() / static_cast<double>(m_ui->tblImages->columnCount()))) };
        m_ui->tblImages->setRowCount(rowCount);
        for(int i = 0; i < rowCount; i++)
        {
            for(int j = 0; j < m_ui->tblImages->columnCount(); j++)
            {
                int index{ i * m_ui->tblImages->columnCount() + j };
                if(index >= m_controller->getSpotlightImages().size())
                {
                    break;
                }
                QPixmap pixmap{ QString::fromStdString(m_controller->getSpotlightImages()[index].string()) };
                QLabel* lbl{ new QLabel() };
                lbl->setPixmap(pixmap.scaled(m_ui->tblImages->horizontalHeader()->defaultSectionSize(), m_ui->tblImages->verticalHeader()->defaultSectionSize(), Qt::KeepAspectRatio, Qt::FastTransformation));
                lbl->setScaledContents(true);
                m_ui->tblImages->setCellWidget(i, j, lbl);
                qApp->processEvents();
            }
        }
        //Setup Flip Page
        m_ui->sliderFlip->setMaximum(m_controller->getSpotlightImages().size() - 1);
        m_ui->sliderFlip->setValue(0);
        onSliderFlipChanged(0);
    }
}