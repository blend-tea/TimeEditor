#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMimeData>
#include <QString>
#include <windows.h>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    connect(ui->fileBrowse, &QPushButton::clicked, this, &MainWindow::file_browse);
    connect(ui->changeTimeButton, &QPushButton::clicked, this, &MainWindow::change_time);

    QStringList args = QCoreApplication::arguments();
    if (args.size() > 1)
    {
        QString filePath = QFileInfo(args[1]).absoluteFilePath();
        QFileInfo fileInfo(filePath);
        if (!fileInfo.isDir())
        {
            ui->filePath->setText(filePath);
            file_selected(filePath);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void MainWindow::file_browse()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*.*)"));

    if (fileName.isEmpty())
    {
        return;
    }

    ui->filePath->setText(fileName);
    file_selected(fileName);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        QList<QUrl> urls = mimeData->urls();

        // Check if multiple files are being dragged
        if (urls.size() > 1)
        {
            QMessageBox::warning(this, "Warning", "Please drop one file.");
            return;
        }

        if (!urls.isEmpty())
        {
            QString filePath = urls.first().toLocalFile();
            QFileInfo fileInfo(filePath);

            // Check if it is not a directory
            if (!fileInfo.isDir())
            {
                ui->filePath->setText(filePath);
                file_selected(filePath);
            }
            else
            {
                QMessageBox::warning(this, "Warning", "Please drop a file.");
            }
        }
    }
}

void MainWindow::file_selected(const QString &filePath)
{
    HANDLE hFile = CreateFile(filePath.toStdWString().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        QMessageBox::critical(this, "Error", "Failed to open the file.");
        return;
    }

    FILETIME creationTime, lastAccessTime, lastWriteTime;
    SYSTEMTIME creationTimeSystem, lastAccessTimeSystem, lastWriteTimeSystem;
    SYSTEMTIME creationTimeJST, lastAccessTimeJST, lastWriteTimeJST;

    if (!GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime))
    {
        CloseHandle(hFile);
        QMessageBox::critical(this, "Error", "Failed to get file time.");
        return;
    }

    if (!CloseHandle(hFile))
    {
        QMessageBox::critical(this, "Error", "Failed to close the file handle.");
    }

    if (!FileTimeToSystemTime(&creationTime, &creationTimeSystem) ||
        !FileTimeToSystemTime(&lastAccessTime, &lastAccessTimeSystem) ||
        !FileTimeToSystemTime(&lastWriteTime, &lastWriteTimeSystem))
    {
        QMessageBox::critical(this, "Error", "Failed to convert file time to system time.");
        return;
    }

    if (!SystemTimeToTzSpecificLocalTime(NULL, &creationTimeSystem, &creationTimeJST) ||
        !SystemTimeToTzSpecificLocalTime(NULL, &lastAccessTimeSystem, &lastAccessTimeJST) ||
        !SystemTimeToTzSpecificLocalTime(NULL, &lastWriteTimeSystem, &lastWriteTimeJST))
    {
        QMessageBox::critical(this, "Error", "Failed to convert system time to local time.");
        return;
    }

    ui->creationTime->setDateTime(
        QDateTime(QDate(creationTimeJST.wYear, creationTimeJST.wMonth, creationTimeJST.wDay),
                  QTime(creationTimeJST.wHour, creationTimeJST.wMinute, creationTimeJST.wSecond)));
    ui->lastAccessTime->setDateTime(
        QDateTime(QDate(lastAccessTimeJST.wYear, lastAccessTimeJST.wMonth, lastAccessTimeJST.wDay),
                  QTime(lastAccessTimeJST.wHour, lastAccessTimeJST.wMinute, lastAccessTimeJST.wSecond)));
    ui->lastWriteTime->setDateTime(
        QDateTime(QDate(lastWriteTimeJST.wYear, lastWriteTimeJST.wMonth, lastWriteTimeJST.wDay),
                  QTime(lastWriteTimeJST.wHour, lastWriteTimeJST.wMinute, lastWriteTimeJST.wSecond)));
}

void MainWindow::change_time()
{
    QDateTime creationTime = ui->creationTime->dateTime();
    QDateTime lastAccessTime = ui->lastAccessTime->dateTime();
    QDateTime lastWriteTime = ui->lastWriteTime->dateTime();

    SYSTEMTIME creationTimeSystemJST, lastAccessTimeSystemJST, lastWriteTimeSystemJST;
    SYSTEMTIME creationTimeSystemUTC, lastAccessTimeSystemUTC, lastWriteTimeSystemUTC;

    creationTimeSystemJST.wYear = creationTime.date().year();
    creationTimeSystemJST.wMonth = creationTime.date().month();
    creationTimeSystemJST.wDay = creationTime.date().day();
    creationTimeSystemJST.wHour = creationTime.time().hour();
    creationTimeSystemJST.wMinute = creationTime.time().minute();
    creationTimeSystemJST.wSecond = creationTime.time().second();
    creationTimeSystemJST.wMilliseconds = 0;

    lastAccessTimeSystemJST.wYear = lastAccessTime.date().year();
    lastAccessTimeSystemJST.wMonth = lastAccessTime.date().month();
    lastAccessTimeSystemJST.wDay = lastAccessTime.date().day();
    lastAccessTimeSystemJST.wHour = lastAccessTime.time().hour();
    lastAccessTimeSystemJST.wMinute = lastAccessTime.time().minute();
    lastAccessTimeSystemJST.wSecond = lastAccessTime.time().second();
    lastAccessTimeSystemJST.wMilliseconds = 0;

    lastWriteTimeSystemJST.wYear = lastWriteTime.date().year();
    lastWriteTimeSystemJST.wMonth = lastWriteTime.date().month();
    lastWriteTimeSystemJST.wDay = lastWriteTime.date().day();
    lastWriteTimeSystemJST.wHour = lastWriteTime.time().hour();
    lastWriteTimeSystemJST.wMinute = lastWriteTime.time().minute();
    lastWriteTimeSystemJST.wSecond = lastWriteTime.time().second();
    lastWriteTimeSystemJST.wMilliseconds = 0;

    TIME_ZONE_INFORMATION timeZoneInfo;
    GetTimeZoneInformation(&timeZoneInfo);

    if (!TzSpecificLocalTimeToSystemTime(&timeZoneInfo, &creationTimeSystemJST, &creationTimeSystemUTC))
    {
        QMessageBox::critical(this, "Error", "Failed to convert creation time to UTC");
        return;
    }
    if (!TzSpecificLocalTimeToSystemTime(&timeZoneInfo, &lastAccessTimeSystemJST, &lastAccessTimeSystemUTC))
    {
        QMessageBox::critical(this, "Error", "Failed to convert last access time to UTC");
        return;
    }
    if (!TzSpecificLocalTimeToSystemTime(&timeZoneInfo, &lastWriteTimeSystemJST, &lastWriteTimeSystemUTC))
    {
        QMessageBox::critical(this, "Error", "Failed to convert last write time to UTC");
        return;
    }

    FILETIME creationTimeFile, lastAccessTimeFile, lastWriteTimeFile;

    if (!SystemTimeToFileTime(&creationTimeSystemUTC, &creationTimeFile))
    {
        QMessageBox::critical(this, "Error", "Failed to convert creation time to FILETIME");
        return;
    }
    if (!SystemTimeToFileTime(&lastAccessTimeSystemUTC, &lastAccessTimeFile))
    {
        QMessageBox::critical(this, "Error", "Failed to convert last access time to FILETIME");
        return;
    }
    if (!SystemTimeToFileTime(&lastWriteTimeSystemUTC, &lastWriteTimeFile))
    {
        QMessageBox::critical(this, "Error", "Failed to convert last write time to FILETIME");
        return;
    }

    HANDLE hFile = CreateFile(ui->filePath->text().toStdWString().c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        QMessageBox::critical(this, "Error", "Failed to open the file.");
        return;
    }

    if (!SetFileTime(hFile, &creationTimeFile, &lastAccessTimeFile, &lastWriteTimeFile))
    {
        CloseHandle(hFile);
        QMessageBox::critical(this, "Error", "Failed to set file time.");
        return;
    }

    if (!CloseHandle(hFile))
    {
        QMessageBox::critical(this, "Error", "Failed to close the file handle.");
    }
    QMessageBox::information(this, "Success", "File time changed successfully.");
}

void MainWindow::on_actionOpen_triggered()
{
    MainWindow::file_browse();
}
