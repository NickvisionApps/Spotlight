#ifndef SPOTLIGHTIMAGE_H
#define SPOTLIGHTIMAGE_H

#include <filesystem>
#include <QLabel>
#include <QMouseEvent>
#include <QSize>

namespace Nickvision::Spotlight::Qt::Controls
{
    /**
     * @brief A control for displaying a Spotlight image.
     */
    class SpotlightImage : public QLabel
    {
    Q_OBJECT

    public:
        /**
         * @brief Constructs a SpotlightImage.
         * @param index The index of the image
         * @param imagePath The path of the image
         * @param width The width of the image
         * @param height The height of the image
         * @param parent The parent widget of the image
         */
        SpotlightImage(int index, const std::filesystem::path& imagePath, int width, int height, QWidget* parent = nullptr);
        /**
         * @brief Gets the index of the image.
         * @return The index of the image
         */
        int index() const;
        /**
         * @brief Gets whether or not the image is selected.
         * @return True if the image is selected
         * @return False if the image is not selected
         */
        bool isSelected() const;
        /**
         * @brief Sets whether or not the image is selected.
         * @param selected Whether or not the image is selected
         */
        void setSelected(bool selected);
        /**
         * @brief Resizes the image.
         * @param width The new width
         * @param height The new height
         */
        void resize(int width, int height);
        /**
         * @brief Resizes the image.
         * @param size The new size
         */
        void resize(const QSize& size);

    Q_SIGNALS:
        /**
         * @brief Emitted when the image is clicked.
         */
        void clicked();

    protected:
        /**
         * @brief Handles when the image's mouse press event.
         * @param event QMouseEvent
         */
        void mousePressEvent(QMouseEvent* event) override;
        /**
         * @brief Handles when the image is double clicked.
         * @param event QMouseEvent
         */
        void mouseDoubleClickEvent(QMouseEvent* event) override;

    private:
        std::filesystem::path m_imagePath;
        int m_index;
        bool m_selected;
    };
}

#endif //SPOTLIGHTIMAGE_H
