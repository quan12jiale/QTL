#ifndef DIALOG_H
#define DIALOG_H

#include <QFileDialog>
#include <QBuffer>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include "sharedmemory.h"

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);

public slots:
    void loadFromFile();
    void loadFromMemory();

private:
    void detach();
    void setupUi();

private:
    SharedMemory sharedMemory;
    QPushButton* loadFromFileButton;
    QPushButton* loadFromSharedMemoryButton;
    QLabel* label;
};

#endif // DIALOG_H
