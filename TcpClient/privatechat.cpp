#include "privatechat.h"
#include "ui_privatechat.h"
#include"protocol.h"
#include"tcpclient.h"
#include<QMessageBox>

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    setWindowTitle("私聊");
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstace()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName = strName;
    m_strLoginName= TcpClient::getInstance().loginName();
    setWindowTitle(QString("与 %1 聊天").arg(strName));
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if(NULL == pdu)
    {
        return ;
    }
    char caLoginName[32] = {'\0'};
    memcpy(caLoginName, pdu->caData + 32, 32);
    QString msg = QString("%1 says: %2").arg(QString::fromUtf8(caLoginName)).arg(QString::fromUtf8((char*)pdu->caMsg));
    ui->showMsg_te->append(msg);
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_le->text();
    if(!strMsg.isEmpty())
    {
        // 先在自己窗口显示发送的消息
        QString msg = QString("%1 says: %2").arg(m_strLoginName).arg(strMsg);
        ui->showMsg_te->append(msg);
        ui->inputMsg_le->clear();
        // 统一转为 UTF-8 字节流，避免中文乱码
        QByteArray utf8Target = m_strChatName.toUtf8();
        QByteArray utf8Sender = m_strLoginName.toUtf8();
        QByteArray utf8Msg = strMsg.toUtf8();
        PDU *pdu = mkPDU(utf8Msg.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData, utf8Target.constData(), qMin(utf8Target.size(), 32));
        memcpy(pdu->caData + 32, utf8Sender.constData(), qMin(utf8Sender.size(), 32));
        memcpy((char*)pdu->caMsg, utf8Msg.constData(), utf8Msg.size() + 1);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this, "私聊", "发送的内容不能为空");
    }
}

void PrivateChat::on_inputMsg_le_returnPressed()
{
    on_sendMsg_pb_clicked();
}
