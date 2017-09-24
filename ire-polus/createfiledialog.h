#ifndef CREATEFILEDIALOG_H
#define CREATEFILEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

class CreateFileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CreateFileDialog(QString& fileName, QStringList& fileList, QWidget *parent = 0);

private slots:
    void accept() override;
    void reject() override;
    void onTextChanged(const QString&);

private:
    QLabel*           _labelName;
    QLineEdit*        _lineEditName;
    QDialogButtonBox* _buttonBox;

    QString&          _fileName;
    QStringList&      _fileList;
};

#endif // CREATEFILEDIALOG_H
