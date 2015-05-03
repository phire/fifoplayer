#include "efb_viewer.h"

#include <arpa/inet.h>
#include <QImageWriter>

EFBViewer::EFBViewer()
{
	efb_color = new QLabel();
	efb_color->setMinimumSize(640, 528);
	this->addWidget(efb_color);


}

void EFBViewer::StartDataReceive(QTcpSocket *socket, u8 cmd)
{
	if(cmd == 0xc0 || cmd == 0xde)
	{
		readType = cmd;
		this->socket = socket;
		readLength = 0;

		connect(socket, SIGNAL(readyRead()), this, SLOT(DataReady()));
		if(socket->bytesAvailable() >= sizeof(readLength))
			DataReady();
	}
}

void EFBViewer::DataReady()
{
	// if we have not already done so, read the length of the image
	if(readLength == 0 && socket->bytesAvailable() >= sizeof(readLength))
	{
		// get the length
		socket->read((char*)&readLength, sizeof(readLength));
		readLength = ntohl(readLength);

		qDebug() << "Starting Efb Read of " << readLength << " Bytes";
	}

	qDebug() << socket->bytesAvailable() << " Buffered";

	// Has the socket buffered the entire image we are expecting?
	if(socket->bytesAvailable() < readLength)
		return;

	qDebug() << "Buffered, reading bytes";

	u32 data[640*528];
	socket->read((char*)data, readLength);

	int static count = 1;

	if(readType == 0xc0)
	{
		QImage image(640, 528, QImage::Format_RGB888);

		for(int y = 0; y < 528; y++)
		{
			for(int x = 0; x < 640; x++)
			{
				u32 c = data[640*y + x];
				// ignore the alpha channel for now
				//image.setPixel(x, y, qRgb((c >> 8) & 0xff, (c >> 16) & 0xff, (c >> 24) & 0xff));
				image.setPixel(x, y, qRgb(c &255, c&255, c&255));
			}
		}
		image.save(QString::fromStdString("efb" + std::to_string(count) + "_alpha.png"));
		efb_color->setPixmap(QPixmap::fromImage(image));
	}
	else
	{
		QImage image(640, 528, QImage::Format_RGB888);

		for(int y = 0; y < 528; y++)
		{
			for(int x = 0; x < 640; x++)
			{
				u32 c = data[640*y + x];
				// middle 8 bits
				image.setPixel(x, y, qRgb((c >> 16) & 0xff, (c >> 16) & 0xff, (c >> 16) & 0xff));
				image.setPixel(x, y, qRgb((c >> 8) & 0xff, (c >> 16) & 0xff, (c >> 24) & 0xff));
			}
		}
		//image.save(QString::fromStdString("efb" + std::to_string(count) + "_depth.png"));
		efb_color->setPixmap(QPixmap::fromImage(image));
		count++;
	}

	// Stop waiting for readyRead signals and notify that we are finished with the socket
	disconnect(socket, SIGNAL(readyRead()), this, SLOT(DataReady()));
	emit ReceiveFinished();
}
