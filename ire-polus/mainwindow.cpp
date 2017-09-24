#include "mainwindow.h"
#include "createfiledialog.h"

#include <QGridLayout>
#include <QWidget>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
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

    fullList(QDir::homePath());
}

void MainWindow::createActions()
{
    _actOpen = new QAction(QIcon(":/images/images/open.png"), tr("&Open"), this);
    _actOpen->setShortcuts(QKeySequence::New);
    _actOpen->setStatusTip(tr("Open directory"));
    connect(_actOpen, &QAction::triggered, this, &MainWindow::openDirectory);

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

    if(!path.isEmpty()) {
        fullList(path);
    }

}

void MainWindow::createFile()
{
    QString fileName;
    QStringList fileList = _dir->entryList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files, QDir::Name | QDir::IgnoreCase);
    CreateFileDialog createFileDialog(fileName, fileList);
    if(createFileDialog.exec() == QDialog::Accepted){
        QFile file(QString("%1/%2").arg(_dir->path()).arg(fileName));
        if(file.open(QIODevice::ReadWrite)){
            file.close();

            fullList(_dir->path());
        }
        else {
            QMessageBox errorDialog;
            errorDialog.setText(QString("Error to create %1 file").arg(fileName));
            errorDialog.setStandardButtons(QMessageBox::Ok);
            errorDialog.exec();
        }

    }
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

            if(!_dir->remove(fileName)){
                // ошибка при удалении файла

                QMessageBox errorDialog;
                errorDialog.setText(QString("Error to remove %1 file").arg(fileName));
                errorDialog.setStandardButtons(QMessageBox::Ok);
                errorDialog.exec();

            }
        }
    }
}

void MainWindow::fullList(const QString& path)
{

    _dir->setPath(path);
    QStringList list = _dir->entryList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files, QDir::Name | QDir::IgnoreCase);

    // clear list
    _fileList->clear();
    _fileList->addItems(list);

    // remove old paths
    QStringList oldList = _fileSystemWatcher->files();
    _fileSystemWatcher->removePaths(oldList);

    // add new paths
    for(auto file : list) {
        _fileSystemWatcher->addPath(QString("%1/%2").arg(path).arg(file));
    }

    _fileSystemWatcher->addPath(path);
}

void MainWindow::onFileChanged(const QString& file)
{
    Q_UNUSED(file)

    fullList(_dir->path());
}

void MainWindow::onDirectoryChanged(const QString& dir)
{
    fullList(dir);
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

MainWindow::~MainWindow()
{

}
