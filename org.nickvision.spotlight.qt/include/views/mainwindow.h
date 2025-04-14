#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QMainWindow>
#include <QTimer>
#include <oclero/qlementine/style/ThemeManager.hpp>
#include "controllers/mainwindowcontroller.h"
#include "controls/spotlightimage.h"

namespace Ui { class MainWindow; }

namespace Nickvision::Spotlight::Qt::Views
{
    /**
     * @brief The main window for the application.
     */
    class MainWindow : public QMainWindow
    {
    Q_OBJECT

    public:
        /**
         * @brief Constructs a MainWindow.
         * @param controller The MainWindowController
         * @param themeManager The theme manager
         * @param parent The parent widget
         */
        MainWindow(const std::shared_ptr<Shared::Controllers::MainWindowController>& controller, oclero::qlementine::ThemeManager* themeManager, QWidget* parent = nullptr);
        /**
         * @brief Destructs a MainWindow.
         */
        ~MainWindow();
        /**
         * @brief Shows the MainWindow.
         */
        void show();

    protected:
        /**
         * @brief Handles when the window is closed.
         * @param event QCloseEvent
         */
        void closeEvent(QCloseEvent* event) override;
        /**
         * @brief Handles when the window is resized.
         * @param event QResizeEvent
         */
        void resizeEvent(QResizeEvent* event) override;

    private Q_SLOTS:
        /**
         * @brief Exports all images.
         */
        void exportAllImages();
        /**
         * @brief Exits the application.
         */
        void exit();
        /**
         * @brief Clears the image cache and syncs the spotlight images.
         */
        void clearAndSync();
        /**
         * @brief Displays the settings dialog.
         */
        void settings();
        /**
         * @brief Exports the selected image.
         */
        void exportImage();
        /**
         * @brief Sets the selected image as the background.
         */
        void setImageAsBackground();
        /**
         * @brief Checks for application updates.
         */
        void checkForUpdates();
        /**
         * @brief Downloads and installs the latest application update in the background.
         */
        void windowsUpdate();
        /**
         * @brief Opens the application's GitHub repo in the browser.
         */
        void gitHubRepo();
        /**
         * @brief Opens the application's issue tracker in the browser.
         */
        void reportABug();
        /**
         * @brief Opens the application's discussions page in the browser.
         */
        void discussions();
        /**
         * @brief Displays the about dialog.
         */
        void about();
        /**
         * @brief Handles when the window is resized.
         */
        void onWindowResize();

    private:
        /**
         * @brief Handles when a notification is sent.
         * @param args The NotificationSentEventArgs
         */
        void onNotificationSent(const Notifications::NotificationSentEventArgs& args);
        /**
         * @brief Handles when the images are synced.
         * @param args ParamEventArgs<std::vector<std::filesystem::path>>
         */
        void onImagesSynced(const Nickvision::Events::ParamEventArgs<std::vector<std::filesystem::path>>& args);
        Ui::MainWindow* m_ui;
        std::shared_ptr<Shared::Controllers::MainWindowController> m_controller;
        oclero::qlementine::ThemeManager* m_themeManager;
        QTimer* m_resizeTimer;
        std::vector<Controls::SpotlightImage*> m_images;
        Controls::SpotlightImage* m_selectedImage;
    };
}

#endif //MAINWINDOW_H
