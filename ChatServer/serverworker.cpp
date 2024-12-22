#include "serverworker.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

ServerWorker::ServerWorker(QObject *parent)
    : QObject{parent}
{
    m_serverSocket =new QTcpSocket(this);
    connect(m_serverSocket,&QTcpSocket::readyRead,this,&ServerWorker::onReadyRead);
    connect(m_serverSocket,&QTcpSocket::disconnected,this,&ServerWorker::disconnectedFromClient);
}

bool ServerWorker::setSocketDescriptor(qintptr socketDescriptor)
{
    return m_serverSocket->setSocketDescriptor(socketDescriptor);
}

QString ServerWorker::userName()
{
    return m_userName;
}

void ServerWorker::setUserName(QString user)
{
    m_userName = user;
}

void ServerWorker::onReadyRead()
{
    QByteArray jsonData;
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_6_7);
    //start an infinite loop
    for(;;){
        socketStream.startTransaction();
        socketStream >>jsonData;
        if(socketStream.commitTransaction()){
            // emit logMessage(QString::fromUtf8(jsonData));
            // sendMessage("I recieved message");
            QJsonParseError parseError;
            const QJsonDocument jsonDoc =QJsonDocument::fromJson(jsonData, &parseError);
            if(parseError.error ==QJsonParseError::NoError){
                if(jsonDoc.isObject()){// and is a JsoN object
                    emit logMessage(QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact));
                    emit jsonReceived(this,jsonDoc.object());// parse the JsON
                }
            }


        } else {
            break;
        }
    }
}

void ServerWorker::sendMessage(const QString &text, const QString &type)
{
    if(m_serverSocket->state()!= QAbstractSocket::ConnectedState)
        return;
    if(!text.isEmpty()){
        //create a QDatastream operating on the socket
        QDataStream serverStream(m_serverSocket);
        serverStream.setVersion(QDataStream::Qt_6_7);

        //Create the JSON we want to send
        QJsonObject message;
        message["type"]= type;
        message["text"]= text;
        // send the JsoN using QDatastream
        serverStream<<QJsonDocument(message).toJson();
    }
}

void ServerWorker::sendJson(const QJsonObject &json)
{
    const QByteArray jsonData =QJsonDocument(json).toJson(QJsonDocument::Compact);
    emit logMessage(QLatin1String("Sending to ")+ userName()+ QLatin1String("-")+QString::fromUtf8(jsonData));
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_6_7);
    socketStream<<jsonData;
}
