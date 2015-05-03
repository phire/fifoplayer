#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTcpSocket>

#include "../source/CommonTypes.h"

class EFBViewer : public QVBoxLayout
{
	Q_OBJECT

public:
	EFBViewer();

public slots:
	void StartDataReceive(QTcpSocket *socket, u8 cmd);
	void DataReady();

signals:
	void ReceiveFinished();

private:
	QLabel* efb_color;
	u8 readType;
	u32 readLength;
	QTcpSocket* socket;
};
