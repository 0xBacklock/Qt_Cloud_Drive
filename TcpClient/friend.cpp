#include "friend.h"
#include"tcpclient.h"
#include"QInputDialog"
#include"privatechat.h"
#include"sharefile.h"

Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_pShowMsgTE = new QTextEdit;
    m_pShowMsgTE->setReadOnly(true);
    m_pFriendListWidget = new QListWidget;
    m_pFriendListWidget->setFixedWidth(180);
    m_pFriendListWidget->setStyleSheet("QListWidget { border: 1px solid #e4e7ed; }");
    m_pInputMsgLE = new QLineEdit;
    m_pInputMsgLE->setPlaceholderText("输入群聊消息...");
    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUserPB = new QPushButton("在线用户");
    m_pSearchUserPB = new QPushButton("查找用户");
    m_pMsgSendPB = new QPushButton("发送");
    m_pPrivateChatPB = new QPushButton("私聊");
    m_pOnline = new Online;

    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->setSpacing(6);
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUserPB);
    pRightPBVBL->addWidget(m_pSearchUserPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);
    pRightPBVBL->addStretch();

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    // 默认隐藏在线用户
    m_pOnline->hide();
    // 显示该布局
    setLayout(pMain);
    // 创建信号链接
    // 点击按钮事件
    connect(m_pShowOnlineUserPB, SIGNAL(clicked(bool)), this, SLOT(showOnline()));
    connect(m_pSearchUserPB, SIGNAL(clicked(bool)), this, SLOT(searchUser()));
    connect(m_pFlushFriendPB, SIGNAL(clicked(bool)), this, SLOT(flushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked(bool)), this, SLOT(deleteFriend()));
    connect(m_pPrivateChatPB, SIGNAL(clicked(bool)), this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked(bool)), this, SLOT(groupChat()));
    connect(m_pInputMsgLE, SIGNAL(returnPressed()), this, SLOT(groupChat()));
}

void Friend::showOnline()
{
    if(m_pOnline->isHidden())
    {
        m_pOnline->show();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
        m_pShowOnlineUserPB->setText("隐藏在线用户");
    }
    else
    {
        m_pOnline->hide();
        m_pShowOnlineUserPB->setText("显示在线用户");
    }
}

void Friend::searchUser()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名：");
    // 输入不为空
    if(!m_strSearchName.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_REQUEST;
        strncpy(pdu->caData, m_strSearchName.toUtf8().constData(), 32);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::flushFriend()
{
    QString loginName = TcpClient::getInstance().loginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    strncpy(pdu->caData, loginName.toUtf8().constData(), 32);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::deleteFriend()
{
    QListWidgetItem *item = m_pFriendListWidget->currentItem();
    if(NULL == item)
    {
        return ;
    }
    QString name = item->text();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
    memcpy(pdu->caData, name.toUtf8().constData(), qMin(name.toUtf8().size(), 32));
    memcpy(pdu->caData + 32, TcpClient::getInstance().loginName().toUtf8().constData(), qMin(TcpClient::getInstance().loginName().toUtf8().size(), 32));
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::privateChat()
{
    QListWidgetItem *item = m_pFriendListWidget->currentItem();
    if(NULL == item)
    {
        QMessageBox::warning(this, "私聊", "请选择私聊对象");
        return ;
    }
    else
    {
        QString friendName = item->text();
        PrivateChat::getInstace().setChatName(friendName);
        if(PrivateChat::getInstace().isHidden())
        {
            PrivateChat::getInstace().show();
        }
    }
}

void Friend::groupChat()
{
    QString msg = m_pInputMsgLE->text();
    if(msg.isEmpty())
    {
        QMessageBox::warning(this, "群聊", "发送消息不能为空");
        return;
    }
    m_pInputMsgLE->clear();
    QByteArray utf8LoginName = TcpClient::getInstance().loginName().toUtf8();
    QByteArray utf8Msg = msg.toUtf8();
    PDU *pdu = mkPDU(utf8Msg.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
    memcpy(pdu->caData, utf8LoginName.constData(), qMin(utf8LoginName.size(), 32));
    memcpy((char *)pdu->caMsg, utf8Msg.constData(), utf8Msg.size() + 1);
    TcpClient::getInstance().getTcpSocket().write((char *)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::showAllOnlineUser(PDU *pdu)
{
    if(NULL == pdu)
    {
        return ;
    }
    m_pOnline->showUser(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if(NULL == pdu)
    {
        return ;
    }
    m_pFriendListWidget->clear();
    uint uiSize = pdu->uiMsgLen / 32;
    for(int i = 0; i < uiSize; i++)
    {
        char caName[32] = {'\0'};
        memcpy(caName, (char*)(pdu->caMsg) + i * 32, 32);
        m_pFriendListWidget->addItem(QString::fromUtf8(caName));
    }
    // 如果分享窗口正打开着，同步更新好友列表
    if(!ShareFile::getInstance().isHidden())
    {
        ShareFile::getInstance().updateFriend(m_pFriendListWidget);
    }
}

void Friend::updateGroupMsg(PDU *pdu)
{
    char caLoginName[32] = {'\0'};
    memcpy(caLoginName, pdu->caData, 32);
    m_pShowMsgTE->append(QString("%1 says: %2").arg(QString::fromUtf8(caLoginName)).arg(QString::fromUtf8((char*)pdu->caMsg)));
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}
