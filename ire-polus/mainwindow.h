#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QFileSystemWatcher>
#include <QAction>
#include <QDir>
#include <QToolBar>

static const int     ICON_SIZE   = 64;
static const QString SERVICE_NAME_SERVER = "ire.polus.server";
static const QString SERVICE_NAME_CLIENT = "ire.polus.client";

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(bool isClient = false, QWidget *parent = 0);
    ~MainWindow();

private slots:
    void openDirectory();                     // открыть директорию для просмотра
    void createFile();                        // создать файл
    void removeFile();                        // удалить файл
    void onFileChanged(const QString&);       // если изменен файл
    void onDirectoryChanged(const QString&);  // если изменена директория
    void onItemSelectionChanged();            // если изменено выделение

    void fullList();       // заполнить список файлами из выбранной директории

public slots:
    Q_SCRIPTABLE bool dbusCreateFile(const QString&);                        // создать файл
    Q_SCRIPTABLE bool dbusRemoveFile(const QString&);                        // удалить файл
    Q_SCRIPTABLE QStringList dbusFullList();
    Q_SCRIPTABLE bool dbusUpdate(const QStringList&);

private:
    void createActions();                     //

private:
    QListWidget* _fileList;
    QDir*        _dir;

    QFileSystemWatcher* _fileSystemWatcher;

    QAction*  _actOpen;
    QAction*  _actRemove;
    QAction*  _actCreate;
    QToolBar* _toolBar;
    bool      _isClient;

    QString   _error;
};


#endif // MAINWINDOW_H
