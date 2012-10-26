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

#ifndef QWIISTREAMTHREAD_H
#define QWIISTREAMTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QFile>

const qint16 HOMEBREW_PORT = 4299;

class QWiiStreamThread : public QThread
{
    Q_OBJECT
public:
    QWiiStreamThread(QString host, QByteArray *data, int size, int size_compressed, QByteArray *args, int args_len);
   ~QWiiStreamThread();

private:
   QString hostname;
   QTcpSocket *tcpSocket;
   QByteArray *dataStream;
   QDataStream *readData;
   int full_size;
   int compressed_size;
   QByteArray *arguments;
   int arguments_size;
   QString errorName;

   qint8 status;
   qint64 readed, total;


private slots:
   void bytesWritten(qint64 value);

public slots:
   void slotConnected();
   void slotError(QAbstractSocket::SocketError socketError);

protected:
   void run();

signals:
   void transferDone();
   void transferFail(QString error);
   void progressBarPosition(int);

};

#endif // QWIISTREAMTHREAD_H
