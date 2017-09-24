#include "mainwindow.h"
#include "createfiledialog.h"

#include <QGridLayout>
#include <QWidget>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QtDBus/QtDBus>
#include <QDBusInterface>
#include <QDBusReply>

#include <QProcess>

MainWindow::MainWindow(bool isClient, QWidget *parent) :
    QMainWindow(parent),
    _isClient(isClient)
{
    createActions();

    _fileList = new QListWidget;

    connect(_fileList, &QListWidget::itemSelectionChanged, this, &MainWindow::onItemSelectionChanged);

    _dir = new QDir;
    _fileSystemWatcher = new QFileSystemWatcher;

    connect(_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::onFileChanged);
    connect(_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &MainWindow::onDirectoryChanged);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(_fileList, 0, 0);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);

    this->setCentralWidget(mainWidget);

    // connect to dbus
    if (!QDBusConnection::sessionBus().isConnected()) {
      _error = QDBusConnection::sessionBus().lastError().message();
    }

    if(_isClient) {

        // регистрируем сервис
        if (!QDBusConnection::sessionBus().registerService(SERVICE_NAME_CLIENT)) {
            _error = QDBusConnection::sessionBus().lastError().message();
        }

        if(!QDBusConnection::sessionBus().registerObject("/", "Client.host", this, QDBusConnection::ExportAllSlots)) {
           _error = QDBusConnection::sessionBus().lastError().message();
        }
    }
    else {
        _dir->setPath(QDir::currentPath());

        // регистрируем сервис
        if (!QDBusConnection::sessionBus().registerService(SERVICE_NAME_SERVER)) {
            _error = QDBusConnection::sessionBus().lastError().message();
        }

        if(!QDBusConnection::sessionBus().registerObject("/", "Server.host", this, QDBusConnection::ExportAllSlots)) {
           _error = QDBusConnection::sessionBus().lastError().message();
        }

        QProcess *myProcess = new QProcess(this);
        myProcess->start(QString("%1/ire-polus").arg(QDir::currentPath()), QStringList("--client"));
    }


    fullList();
}

MainWindow::~MainWindow()
{

}

void MainWindow::createActions()
{
    _actOpen = new QAction(QIcon(":/images/images/open.png"), tr("&Open"), this);
    _actOpen->setShortcuts(QKeySequence::New);
    _actOpen->setStatusTip(tr("Open directory"));
    connect(_actOpen, &QAction::triggered, this, &MainWindow::openDirectory);

    if(_isClient) {
        _actOpen->setEnabled(false);
    }

    _actCreate = new QAction(QIcon(":/images/images/add.png"), tr("&Create file"), this);
    _actCreate->setShortcuts(QKeySequence::New);
    _actCreate->setStatusTip(tr("Add file"));
    connect(_actCreate, &QAction::triggered, this, &MainWindow::createFile);

    _actRemove = new QAction(QIcon(":/images/images/remove.png"), tr("&Remove file"), this);
    _actRemove->setShortcuts(QKeySequence::New);
    _actRemove->setStatusTip(tr("Remove file"));
    _actRemove->setEnabled(false);
    connect(_actRemove, &QAction::triggered, this, &MainWindow::removeFile);


    // create toolbar

    QList<QToolBar *> allToolBars = this->findChildren<QToolBar *>();
    foreach(QToolBar *tb, allToolBars) {
        // This does not delete the tool bar.
        this->removeToolBar(tb);
    }

    _toolBar = addToolBar(tr("File"));
    _toolBar->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    _toolBar->addAction(_actOpen);
    _toolBar->addAction(_actCreate);
    _toolBar->addAction(_actRemove);
}

void MainWindow::openDirectory()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 _dir->path(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    if(path.isEmpty()) {
        return;
    }

    _dir->setPath(path);


    if(!_isClient) {
        QDBusInterface iface(SERVICE_NAME_CLIENT, "/", "Client.host", QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QStringList fileList = dbusFullList();
            QDBusReply<bool> reply = iface.call("dbusUpdate", fileList);
            if (reply.isValid()) {
                bool result = reply.value();
                if(!result) {
                    _error = "Client dont updated";
                }
            }
        }

    }

    fullList();

}


void MainWindow::createFile()
{
    QString fileName;
    QStringList fileList;

    if(_isClient) {
        QDBusInterface iface(SERVICE_NAME_SERVER, "/", "Server.host", QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<QStringList> reply = iface.call("dbusFullList");
            if (reply.isValid()) {
                fileList = reply.value();

            }
        }
    }
    else {
        fileList = dbusFullList();
    }

    CreateFileDialog createFileDialog(fileName, fileList);
    if(createFileDialog.exec() == QDialog::Accepted){

        if(_isClient) {
            QDBusInterface iface(SERVICE_NAME_SERVER, "/", "Server.host", QDBusConnection::sessionBus());
            if (iface.isValid()) {
                QDBusReply<bool> reply = iface.call("dbusCreateFile", fileName);
                if (reply.isValid()) {
                    bool result = reply.value();

                    if(result) {
                        fullList();
                    }
                    else {
                        QMessageBox errorDialog;
                        errorDialog.setText(QString("Error to create %1 file").arg(fileName));
                        errorDialog.setStandardButtons(QMessageBox::Ok);
                        errorDialog.exec();
                    }

                }
            }
        }
        else {
            if(!dbusCreateFile(fileName)) {
                QMessageBox errorDialog;
                errorDialog.setText(QString("Error to create %1 file").arg(fileName));
                errorDialog.setStandardButtons(QMessageBox::Ok);
                errorDialog.exec();
            }

            QDBusInterface iface(SERVICE_NAME_CLIENT, "/", "Client.host", QDBusConnection::sessionBus());
            if (iface.isValid()) {
                QStringList fileList = dbusFullList();
                QDBusReply<bool> reply = iface.call("dbusUpdate", fileList);
                if (reply.isValid()) {
                    bool result = reply.value();
                    if(!result) {
                        _error = "Client dont updated";
                    }
                }
            }
        }

        fullList();

    }
}

bool MainWindow::dbusCreateFile(const QString& fileName)
{
    QFile file(QString("%1/%2").arg(_dir->path()).arg(fileName));
    if(!file.open(QIODevice::ReadWrite)){
        return false;
    }

    file.close();

    return true;
}

void MainWindow::removeFile()
{

        QList<QListWidgetItem *> selectedItems = _fileList->selectedItems();

        if(!selectedItems.isEmpty()) {

            QString fileName = selectedItems.first()->text();

            QMessageBox confirmDialog;
            confirmDialog.setText(QString("Do you want to remove %1 file?").arg(fileName));
            confirmDialog.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
            if(confirmDialog.exec() == QMessageBox::Yes){
                // подтверждение намерения удалить файл

                if(_isClient) {

                    QDBusInterface iface(SERVICE_NAME_SERVER, "/", "Server.host", QDBusConnection::sessionBus());
                    if (iface.isValid()) {
                        QDBusReply<bool> reply = iface.call("dbusRemoveFile", fileName);
                        if (reply.isValid()) {
                            bool result = reply.value();

                            if(!result) {
                                QMessageBox errorDialog;
                                errorDialog.setText(QString("Error to remove %1 file").arg(fileName));
                                errorDialog.setStandardButtons(QMessageBox::Ok);
                                errorDialog.exec();
                            }
                        }
                    }
                }
                else {
                    if(!dbusRemoveFile(fileName)) {
                        QMessageBox errorDialog;
                        errorDialog.setText(QString("Error to remove %1 file").arg(fileName));
                        errorDialog.setStandardButtons(QMessageBox::Ok);
                        errorDialog.exec();
                    }

                    QDBusInterface iface(SERVICE_NAME_CLIENT, "/", "Client.host", QDBusConnection::sessionBus());
                    if (iface.isValid()) {
                        QStringList fileList = dbusFullList();
                        QDBusReply<bool> reply = iface.call("dbusUpdate", fileList);
                        if (reply.isValid()) {
                            bool result = reply.value();
                            if(!result) {
                                _error = "Client dont updated";
                            }
                        }
                    }
                }

            }

            fullList();


        }



}

bool MainWindow::dbusRemoveFile(const QString& fileName)
{

    if(!_dir->remove(fileName)){
        // ошибка при удалении файла
        return false;

    }

    return true;
}

void MainWindow::fullList()
{
    if(_isClient) {
        QDBusInterface iface(SERVICE_NAME_SERVER, "/", "Server.host", QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<QStringList> reply = iface.call("dbusFullList");
            if (reply.isValid()) {
                QStringList result = reply.value();

                // clear list
                _fileList->clear();
                _fileList->addItems(result);
            }
        }
    }
    else {
         QStringList fileList = dbusFullList();

        // clear list
        _fileList->clear();
        _fileList->addItems(fileList);

        // remove old paths
        QStringList oldList = _fileSystemWatcher->files();
        _fileSystemWatcher->removePaths(oldList);

        // add new paths
        for(auto file : fileList) {
            _fileSystemWatcher->addPath(QString("%1/%2").arg(_dir->path()).arg(file));
        }

        _fileSystemWatcher->addPath(_dir->path());
    }

}

QStringList MainWindow::dbusFullList()
{

    QStringList fileList = _dir->entryList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files, QDir::Name | QDir::IgnoreCase);

    return fileList;
}

bool MainWindow::dbusUpdate(const QStringList& fileList)
{
    _fileList->clear();
    _fileList->addItems(fileList);

    return true;
}

void MainWindow::onFileChanged(const QString& file)
{
    Q_UNUSED(file)

    fullList();
}

void MainWindow::onDirectoryChanged(const QString& dir)
{
    _dir->setPath(dir);
    fullList();
}

void MainWindow::onItemSelectionChanged()
{
    QList<QListWidgetItem *> selectedItems = _fileList->selectedItems();

    if(selectedItems.isEmpty()) {
        _actRemove->setEnabled(false);
    }
    else {
        _actRemove->setEnabled(true);
    }
}


