#include "dialog.h"

Dialog::Dialog(QWidget *parent)
  : QDialog(parent), sharedMemory("Glodon")
{
    setupUi();
    connect(loadFromFileButton, SIGNAL(clicked()), SLOT(loadFromFile()));
    connect(loadFromSharedMemoryButton,
            SIGNAL(clicked()),
            SLOT(loadFromMemory()));
    setWindowTitle(tr("SharedMemory Example"));
}

void Dialog::loadFromFile()
{
    if (sharedMemory.isAttached())
        detach();

    label->setText(tr("Select an image file"));
    QString fileName = QFileDialog::getOpenFileName(nullptr, QString(), QString(),
                                        tr("Images (*.png *.xpm *.jpg)"));
    QImage image;
    if (!image.load(fileName)) {
        label->setText(tr("Selected file is not an image, please select another."));
        return;
    }
    label->setPixmap(QPixmap::fromImage(image));

    // load into shared memory
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);
    out << image;
    int size = static_cast<int>(buffer.size());

    if (!sharedMemory.create(size)) {
        label->setText(tr("Unable to create shared memory segment."));
        return;
    }
    sharedMemory.lock();
    char *to = reinterpret_cast<char*>(sharedMemory.data());
    const char *from = buffer.data().data();
    qDebug() << "sharedMemory.size() =" << sharedMemory.size() << "size =" << size;
    size_t len = static_cast<size_t>(qMin(sharedMemory.size(), size));
    memcpy(to, from, len);
    sharedMemory.unlock();
}


void Dialog::loadFromMemory()
{
    if (!sharedMemory.attach()) {
        label->setText(tr("Unable to attach to shared memory segment.\n" \
                             "Load an image first."));
        return;
    }

    QBuffer buffer;
    QDataStream in(&buffer);
    QImage image;

    sharedMemory.lock();
    char* data = reinterpret_cast<char*>(const_cast<void*>(sharedMemory.constData()));
    qDebug() << "sharedMemory.size() =" << sharedMemory.size();
    buffer.setData(data, sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);
    in >> image;
    sharedMemory.unlock();

    sharedMemory.detach();
    label->setPixmap(QPixmap::fromImage(image));
}

void Dialog::detach()
{
    if (!sharedMemory.detach())
        label->setText(tr("Unable to detach from shared memory."));
}

void Dialog::setupUi()
{
    loadFromFileButton = new QPushButton(this);
    loadFromFileButton->setText("Load Image From File...");

    loadFromSharedMemoryButton = new QPushButton(this);
    loadFromSharedMemoryButton->setText("Display Image From Shared Memory");

    label = new QLabel(this);
    label->setWordWrap(true);
    label->setText("Launch two of these dialogs.  "
                   "In the first, press the top button and load an image from a file.  "
                   "In the second, press the bottom button and display the loaded image from shared memory.");

    QVBoxLayout* pLayout = new QVBoxLayout(this);
    pLayout->setSpacing(15);
    pLayout->addWidget(loadFromFileButton);
    pLayout->addWidget(label);
    pLayout->addWidget(loadFromSharedMemoryButton);
}
