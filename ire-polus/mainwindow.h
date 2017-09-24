#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QFileSystemWatcher>
#include <QAction>
#include <QDir>
#include <QToolBar>

static const int              ICON_SIZE   = 64;
static const QDir::Filters    DIR_FILTER  = QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files;
static const QDir::SortFlags  SORT_FILTER = QDir::Name | QDir::IgnoreCase;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void openDirectory();
    void createFile();
    void removeFile();

    void fullList(const QString& path);
    void onFileChanged(const QString&);
    void onDirectoryChanged(const QString&);
    void onItemSelectionChanged();

private:
    void createActions();

private:
    QListWidget* _fileList;
    QDir*        _dir;

    QFileSystemWatcher* _fileSystemWatcher;

    QAction*  _actOpen;
    QAction*  _actRemove;
    QAction*  _actCreate;
    QToolBar* _toolBar;
};


#endif // MAINWINDOW_H
