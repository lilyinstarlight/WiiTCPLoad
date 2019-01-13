/***************************************************************************
 *   Copyright (C) 2008 by Bartlomiej Burdukiewicz                         *
 *   dev.strikeu@gmail.com                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QByteArray>

const QString FILE_NOT_FOUND = "%1: File not found !";
const QString CANT_OPEN_TO_READ = "%1: Cannot open file to read !";
const QString FILE_IS_EMPTY = "%1: File is empty !";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    QDir *directory = new QDir;

    appPath = QDir::homePath() + QString("/.wiitcpload/");
    configPath = appPath + QString("config.ini");

    if (directory->exists(appPath) != true)
    {
        if (directory->mkdir(appPath) != true)
        {
            QMessageBox::critical(this, tr("Critical"), QString("Can't create %1 directory").arg(appPath));
            return;
        }
    }

    delete directory;

    ui->setupUi(this);

    if (QCoreApplication::arguments().count() > 1)
        ui->fileEdit->setText(QCoreApplication::arguments().at(1));


    QSettings *configFile = new QSettings(configPath, QSettings::IniFormat);

    configFile->beginGroup("settings");
    ui->hostEdit->setText(configFile->value("Hostname", "").toString());
    if (ui->fileEdit->text().count() == 0)
        ui->fileEdit->setText(configFile->value("Filename", "").toString());
    if (ui->argEdit->text().count() == 0)
        ui->argEdit->setText(configFile->value("Arguments", "").toString());
    configFile->endGroup();

    delete configFile;

    ui->actionQuit->setShortcut(QKeySequence::Close);

    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(actionQuit()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(actionAbout()));
    connect(ui->openFile, SIGNAL(clicked()), this, SLOT(openFile()));
}

MainWindow::~MainWindow()
{
    QSettings *configFile = new QSettings(configPath, QSettings::IniFormat);

    configFile->beginGroup("settings");
    configFile->setValue("Hostname", ui->hostEdit->text());
    configFile->setValue("Filename", ui->fileEdit->text());
    configFile->setValue("Arguments", ui->argEdit->text());
    configFile->endGroup();

    delete configFile;

    delete ui;
}

void MainWindow::actionAbout()
{
    aboutForm = new AboutForm(this);
    aboutForm->exec();
    delete aboutForm;
}

void MainWindow::openFile()
{
    QFileDialog *fileOpenDialog = new QFileDialog(this);
    QString fileName = fileOpenDialog->getOpenFileName();
    if (fileName.count() != 0)
        ui->fileEdit->setText(fileName);
    delete fileOpenDialog;

}

void MainWindow::progressBarPosition(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::transferDone()
{
    QMessageBox::information(this, "Information", "Data written successful");
    ui->streamButton->setEnabled(true);
    ui->progressBar->setEnabled(false);
    ui->progressBar->setValue(0);
}

void MainWindow::transferFail(QString errorName)
{
    QMessageBox::critical(this, "Critical", errorName);
    ui->streamButton->setEnabled(true);
    ui->progressBar->setEnabled(false);
    ui->progressBar->setValue(0);
}

void MainWindow::on_streamButton_clicked()
{
    if (QFile::exists(ui->fileEdit->text()) != true)
    {
        QMessageBox::critical(this, "Critical", QString(FILE_NOT_FOUND).arg(ui->fileEdit->text()));
        return;
    }

    QFile *file = new QFile(ui->fileEdit->text());
    if (!file->open(QIODevice::ReadOnly))
    {
        delete file;
        QMessageBox::critical(this, "Critical", QString(CANT_OPEN_TO_READ).arg(ui->fileEdit->text()));
        return;
    }

    if (file->size() == 0)
    {
        file->close();
        delete file;
        QMessageBox::critical(this, "Critical", QString(FILE_IS_EMPTY).arg(ui->fileEdit->text()));
    }

    QByteArray *data = new QByteArray();
    *data = file->readAll();
    file->close();
    delete file;

    QByteArray *data_compressed = new QByteArray();
    *data_compressed = qCompress(*data).remove(0, 4);

    int size = data->size();
    int size_compressed = data_compressed->size();
    if(size_compressed < size) {
        delete data;
        data = data_compressed;
    }
    else {
        delete data_compressed;
        size_compressed = size;
    }

    QByteArray *args = new QByteArray();
    //*args = ui->argEdit->text().toAscii(); //toAscii() has been depreciated in qt5
    *args = ui->argEdit->text().toLatin1(); //toAscii() has been replaced by toLatin1()
    int args_len = 0;
    if(!args->isEmpty())
        args_len = args->size() + 1;

    ui->progressBar->setMaximum(size_compressed);
    ui->progressBar->setEnabled(true);

    wiiStreamThread = new QWiiStreamThread(ui->hostEdit->text(), data, size, size_compressed, args, args_len);
    connect(wiiStreamThread, SIGNAL(transferFail(QString)), this, SLOT(transferFail(QString)));
    connect(wiiStreamThread, SIGNAL(transferDone()), this, SLOT(transferDone()));
    connect(wiiStreamThread, SIGNAL(progressBarPosition(int)), this, SLOT(progressBarPosition(int)));
    wiiStreamThread->start();
    ui->streamButton->setEnabled(false);
}
