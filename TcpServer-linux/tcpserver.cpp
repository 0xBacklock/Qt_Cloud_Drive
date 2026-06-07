#include "tcpserver.h"
#include "ui_tcpserver.h"
#include <QLabel>
#include <QCoreApplication>
#include <QFile>

TcpServer::TcpServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    setWindowTitle("云盘服务器");
    resize(400, 250);
    setStyleSheet("background: #f5f7fa;");

    // 加载配置文件
    loadConfig();
    // 服务端开启监听端口链接
    MyTcpServer::getInstance().listen(QHostAddress(m_strIp), m_usPort);

    QLabel *pStatusLabel = new QLabel(this);
    pStatusLabel->setText(QString("服务器已启动\n监听地址: %1:%2").arg(m_strIp).arg(m_usPort));
    pStatusLabel->setAlignment(Qt::AlignCenter);
    pStatusLabel->setStyleSheet("font-size: 16px; color: #409eff; font-weight: bold; background: transparent;");
    pStatusLabel->setGeometry(0, 0, 400, 250);
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    // 从可执行文件同目录的 server.config 读取，不存在则用默认值
    QFile file(QCoreApplication::applicationDirPath() + "/server.config");
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray btData = file.readAll();
        QString fileStrData = QString::fromUtf8(btData);
        fileStrData.replace("\r\n", " ");
        fileStrData.replace("\n", " ");
        QStringList strList = fileStrData.split(" ", Qt::SkipEmptyParts);
        m_strIp = strList.value(0, "0.0.0.0");
        m_usPort = strList.value(1, "8888").toUShort();
        qDebug() << "加载配置:" << m_strIp << m_usPort;
        file.close();
    } else {
        m_strIp = "0.0.0.0";
        m_usPort = 8888;
        qDebug() << "配置不存在，使用默认:" << m_strIp << m_usPort;
    }
}
