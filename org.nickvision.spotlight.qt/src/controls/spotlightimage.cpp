#include "controls/spotlightimage.h"

namespace Nickvision::Spotlight::Qt::Controls
{
    SpotlightImage::SpotlightImage(int index, const std::filesystem::path& imagePath, int width, int height, QWidget* parent)
        : QLabel{ parent },
        m_imagePath{ imagePath },
        m_index{ index },
        m_selected{ false }
    {
        setMinimumSize(width, height);
        setScaledContents(true);
        resize(width, height);
    }

    int SpotlightImage::index() const
    {
        return m_index;
    }

    bool SpotlightImage::isSelected() const
    {
        return m_selected;
    }

    void SpotlightImage::setSelected(bool selected)
    {
        m_selected = selected;
        setFrameStyle(m_selected ? QFrame::Panel | QFrame::Raised : QFrame::Plain);
        setLineWidth(m_selected ? 2 : 1);
    }

    void SpotlightImage::resize(int width, int height)
    {
        resize({ width, height });
    }

    void SpotlightImage::resize(const QSize& size)
    {
        QPixmap pixmap{ QString::fromStdString(m_imagePath.string()) };
        setPixmap(pixmap.scaled(size, ::Qt::KeepAspectRatio, ::Qt::FastTransformation));
    }

    void SpotlightImage::mousePressEvent(QMouseEvent* event)
    {
        Q_EMIT clicked();
    }
}
