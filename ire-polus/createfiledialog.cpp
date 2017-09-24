#include "createfiledialog.h"

#include <QVBoxLayout>
#include <QPushButton>

CreateFileDialog::CreateFileDialog(QString& fileName, QStringList& fileList, QWidget *parent)
    : QDialog(parent),
      _fileName(fileName),
      _fileList(fileList)
{
    _labelName = new QLabel("Name: ");

    _lineEditName = new QLineEdit;

    connect(_lineEditName, &QLineEdit::textChanged, this, &CreateFileDialog::onTextChanged);

    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    _buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(_buttonBox, &QDialogButtonBox::accepted, this, &CreateFileDialog::accept);
    connect(_buttonBox, &QDialogButtonBox::rejected, this, &CreateFileDialog::reject);

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->addWidget(_labelName);
    nameLayout->addWidget(_lineEditName);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(nameLayout);
    mainLayout->addWidget(_buttonBox);

    this->setLayout(mainLayout);
}

void CreateFileDialog::accept()
{
    _fileName = _lineEditName->text();

    QDialog::accept();
}

void CreateFileDialog::reject()
{
    QDialog::reject();
}

void CreateFileDialog::onTextChanged(const QString& text)
{
    if(text.isEmpty() || _fileList.contains(text)) {
        // если не ввели имя файла или файл с таким именем уже существует
        _buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else {
        _buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }

}
