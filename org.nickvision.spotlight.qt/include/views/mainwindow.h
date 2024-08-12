#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QCloseEvent>
#include <QMainWindow>
#include <QResizeEvent>
#include <QTimer>
#include "controllers/mainwindowcontroller.h"

namespace Ui { class MainWindow; }

namespace Nickvision::Spotlight::QT::Views
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
         * @param parent The parent widget
         */
        MainWindow(const std::shared_ptr<Shared::Controllers::MainWindowController>& controller, QWidget* parent = nullptr);
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
         * @brief Exports the selected image.
         */
        void exportImage();
        /**
         * @brief Exports all images.
         */
        void exportAllImages();
        /**
         * @brief Exits the application.
         */
        void exit();
        /**
         * @brief Displays the settings dialog.
         */
        void settings();
        /**
         * @brief Switches to the grid view mode.
         * @param toggled True to switch, false to ignore
         */
        void gridMode(bool toggled);
        /**
         * @brief Switches to the flip view mode.
         * @param toggled True to switch, false to ignore
         */
        void flipMode(bool toggled);
        /**
         * @brief Sets the selected image as the background.
         */
        void setImageAsBackground();
        /**
         * @brief Checks for application updates.
         */
        void checkForUpdates();
#ifdef _WIN32
        /**
         * @brief Downloads and installs the latest application update in the background.
         */
        void windowsUpdate();
#endif
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
         * @brief Loads spotlight images into a grid view.
         */
        void loadGridView();
        /**
         * @brief Handles when the tblImages' selection is changed.
         * @param row The row index
         * @param column The column index
         */
        void onTblImagesSelectionChanged(int row, int column);
        /**
         * @brief Handles when the tblImages is double clicked.
         * @param row The row index
         * @param column The column index
         */
        void onTblImagesDoubleClicked(int row, int column);
        /**
         * @brief Handles when the flip view's slider is changed.
         * @param value The value of the slider
         */
        void onSliderFlipChanged(int value);
        /**
         * @brief Flips to the next image.
         */
        void flipNext();
        /**
         * @brief Flips to the previous image.
         */
        void flipPrev();

    private:
        /**
         * @brief Handles when a notification is sent.
         * @param args The NotificationSentEventArgs
         */
        void onNotificationSent(const Notifications::NotificationSentEventArgs& args);
        /**
         * @brief Handles when a shell notification is sent.
         * @param args The ShellNotificationSentEventArgs
         */
        void onShellNotificationSent(const Notifications::ShellNotificationSentEventArgs& args);
        /**
         * @brief Handles when the images are synced.
         */
        void onImagesSynced();
        Ui::MainWindow* m_ui;
        std::shared_ptr<Shared::Controllers::MainWindowController> m_controller;
        QTimer m_resizeTimer;
    };
}

#endif //MAINWINDOW_H