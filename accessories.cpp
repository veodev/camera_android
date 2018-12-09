#include <QTime>
#include <QCoreApplication>
#include "accessories.h"

void delay(int msToWait)
{
    QTime stopTime = QTime::currentTime().addMSecs(msToWait);
    while (QTime::currentTime() < stopTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}
