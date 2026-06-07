#include "book.h"
#include"tcpclient.h"
#include"QInputDialog"
#include"QMessageBox"
#include<QFileDialog>
#include<QFile>
#include<QLabel>
#include"sharefile.h"

Book::Book(QWidget *parent) : QWidget(parent)
{
    m_pDownload = false;
    m_pTimer = new QTimer;
    m_pBookListW = new QListWidget;
    m_pBookListW->setStyleSheet("QListWidget { border: 1px solid #e4e7ed; }");
    m_pReturnPB = new QPushButton("返回上级");
    m_pCreateDirPB = new QPushButton("新建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenameDirPB = new QPushButton("重命名");
    m_pFlushDirPB = new QPushButton("刷新");
    m_pUploadFilePB = new QPushButton("上传文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pDownloadFilePB = new QPushButton("下载文件");
    m_pShareFilePB = new QPushButton("分享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pMoveSelectDirPB = new QPushButton("选择目标目录");
    m_pMoveSelectDirPB->setEnabled(false);

    QLabel *pDirLabel = new QLabel("目录操作");
    pDirLabel->setStyleSheet("font-weight: bold; color: #606266; padding: 4px 0;");
    QLabel *pFileLabel = new QLabel("文件操作");
    pFileLabel->setStyleSheet("font-weight: bold; color: #606266; padding: 4px 0;");

    QVBoxLayout *dirLayout = new QVBoxLayout;
    dirLayout->setSpacing(6);
    dirLayout->addWidget(pDirLabel);
    dirLayout->addWidget(m_pReturnPB);
    dirLayout->addWidget(m_pCreateDirPB);
    dirLayout->addWidget(m_pDelDirPB);
    dirLayout->addWidget(m_pRenameDirPB);
    dirLayout->addWidget(m_pFlushDirPB);
    dirLayout->addStretch();

    QVBoxLayout *fileLayout = new QVBoxLayout;
    fileLayout->setSpacing(6);
    fileLayout->addWidget(pFileLabel);
    fileLayout->addWidget(m_pUploadFilePB);
    fileLayout->addWidget(m_pDelFilePB);
    fileLayout->addWidget(m_pDownloadFilePB);
    fileLayout->addWidget(m_pShareFilePB);
    fileLayout->addWidget(m_pMoveFilePB);
    fileLayout->addWidget(m_pMoveSelectDirPB);
    fileLayout->addStretch();

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(8, 8, 8, 8);
    hBoxLayout->setSpacing(10);
    hBoxLayout->addWidget(m_pBookListW);
    hBoxLayout->addLayout(dirLayout);
    hBoxLayout->addLayout(fileLayout);

    setLayout(hBoxLayout);
    // 槽与信号
    connect(m_pCreateDirPB, SIGNAL(clicked(bool)), this, SLOT(createDir()));
    connect(m_pFlushDirPB, SIGNAL(clicked(bool)), this, SLOT(flushDir()));
    connect(m_pDelDirPB, SIGNAL(clicked(bool)), this, SLOT(delDir()));
    connect(m_pRenameDirPB, SIGNAL(clicked(bool)), this, SLOT(renameDir()));
    connect(m_pBookListW, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(enterDir(QModelIndex)));
    connect(m_pReturnPB, SIGNAL(clicked(bool)),
            this, SLOT(returnPre()));
    connect(m_pUploadFilePB, SIGNAL(clicked(bool)),
            this, SLOT(uploadPre()));
    connect(m_pTimer, SIGNAL(timeout()),
            this, SLOT(uploadFileData()));
    connect(m_pDelFilePB, SIGNAL(clicked(bool)),
            this, SLOT(delFile()));
    connect(m_pDownloadFilePB, SIGNAL(clicked(bool)),
            this, SLOT(downloadFile()));
    connect(m_pShareFilePB, SIGNAL(clicked(bool)),
            this, SLOT(shareFile()));
    connect(m_pMoveFilePB, SIGNAL(clicked(bool)),
            this, SLOT(moveFile()));
    connect(m_pMoveSelectDirPB, SIGNAL(clicked(bool)),
            this, SLOT(selectDestDir()));

}

void Book::updateDirList(const PDU *pdu)
{
    if(NULL == pdu)
    {
        return ;
    }
    // 清除之前的列表，避免这里的add导致重复，也可以使用for循环 m_pBookListW->item(rowIdx)来进行删除每一行
    m_pBookListW->clear();
    FileInfo *pFileInfo = NULL;
    int iFileCount = pdu->uiMsgLen / sizeof(FileInfo);
    for(int i = 0; i < iFileCount; i++)
    {
        pFileInfo = (FileInfo *)(pdu->caMsg) + i;
        QListWidgetItem *pItem = new QListWidgetItem;
        if(0 == pFileInfo->iFileType)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/dir.png")));
        }
        else if(1 == pFileInfo->iFileType)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/reg.jpg")));
        }
        pItem->setText(QString::fromUtf8(pFileInfo->caFileName));
        m_pBookListW->addItem(pItem);
    }
}

QString Book::getEnterPath()
{
    return m_enterPath;
}

void Book::setDownloadStatus(bool status)
{
    m_pDownload = status;
}

bool Book::getDownloadStatus()
{
    return m_pDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

void Book::updateLocalDownloadFileName()
{
    m_pFile.setFileName(m_strSaveFilePath);
}

QString Book::getShareFileName()
{
    return m_shareFileName;
}

void Book::createDir()
{
    QString strDirName = QInputDialog::getText(this, "新建文件夹", "新文件夹名称");
    if(strDirName.isEmpty())
    {
        QMessageBox::warning(this, "新建文件夹", "新文件夹名称不能为空");
        return ;
    }
    if(strDirName.size() > 32)
    {
        QMessageBox::warning(this, "新建文件夹", "新文件夹名称不能超过32个字符");
        return ;
    }
    QString strLoginName = TcpClient::getInstance().loginName();
    QString strCurPath = TcpClient::getInstance().curPath();
    QByteArray utf8CurPath = strCurPath.toUtf8();
    PDU *pdu = mkPDU(utf8CurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
    // 用户名、新文件夹名称放在caData， 当前路径放在caMsg
    strncpy(pdu->caData, strLoginName.toUtf8().constData(), 32);
    strncpy(pdu->caData + 32, strDirName.toUtf8().constData(), 32);
    memcpy((char *)pdu->caMsg, utf8CurPath.constData(), utf8CurPath.size() + 1);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::flushDir()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QByteArray utf8CurPath = strCurPath.toUtf8();
    PDU *pdu = mkPDU(utf8CurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_REQUEST;
    memcpy((char*)pdu->caMsg, utf8CurPath.constData(), utf8CurPath.size() + 1);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::delDir()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
        return ;
    }
    QString strDelName = pItem->text();
    QByteArray utf8CurPath = strCurPath.toUtf8();
    PDU *pdu = mkPDU(utf8CurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
    strncpy(pdu->caData, strDelName.toUtf8().constData(), 32);
    memcpy((char*)pdu->caMsg, utf8CurPath.constData(), utf8CurPath.size() + 1);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::renameDir()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "重命名文件", "请选择要重命名的文件");
        return ;
    }
    QString strOldName = pItem->text();
    QString strNewName = QInputDialog::getText(this, "重命名文件", "请输入新的文件名");
    if(strNewName.isEmpty())
    {
        QMessageBox::warning(this, "重命名文件", "新文件名不能为空");
        return ;
    }
    QByteArray utf8CurPath = strCurPath.toUtf8();
    PDU *pdu = mkPDU(utf8CurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_DIR_REQUEST;
    strncpy(pdu->caData, strOldName.toUtf8().constData(), 32);
    strncpy(pdu->caData + 32, strNewName.toUtf8().constData(), 32);
    memcpy((char*)pdu->caMsg, utf8CurPath.constData(), utf8CurPath.size() + 1);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::enterDir(const QModelIndex &index)
{
    QString strDirName = index.data().toString();
    QString strCurPath = TcpClient::getInstance().curPath();
    // 缓存数据
    m_enterPath = QString("%1/%2").arg(strCurPath).arg(strDirName);
    QByteArray utf8CurPath = strCurPath.toUtf8();
    PDU *pdu = mkPDU(utf8CurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData, strDirName.toUtf8().constData(), 32);
    memcpy((char*)pdu->caMsg, utf8CurPath.constData(), utf8CurPath.size() + 1);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::returnPre()
{
    // 当前目录
    QString strCurPath = TcpClient::getInstance().curPath();
    // 用户根目录
    QString strRootPath = "./" + TcpClient::getInstance().loginName();
    // 如果当前目录就是用户根目录，代表没有上级目录了
    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this, "返回", "返回上一级失败：当前已经在用户根目录了");
        return ;
    }
    // ./alice/aa/bb -> ./alice/aa
    int idx = strCurPath.lastIndexOf('/');
    strCurPath.remove(idx, strCurPath.size()-idx);
    // 更新当前所在目录位置
    TcpClient::getInstance().setCurPath(strCurPath);
    // 刷新目录文件为当前目录位置的文件
    flushDir();
}

void Book::uploadPre()
{
    // 当前目录
    QString strCurPath = TcpClient::getInstance().curPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if(m_strUploadFilePath.isEmpty())
    {
        QMessageBox::warning(this, "上传文件", "上传文件名称不能为空");
        return ;
    }
    int idx = m_strUploadFilePath.lastIndexOf('/');
    QString fileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - idx - 1);
    QFile file(m_strUploadFilePath);
    qint64 fileSize = file.size();//文件的大小
    QByteArray utf8CurPath = strCurPath.toUtf8();
    PDU *pdu = mkPDU(utf8CurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
    memcpy((char*)pdu->caMsg, utf8CurPath.constData(), utf8CurPath.size() + 1);
    sprintf(pdu->caData, "%s %lld", fileName.toUtf8().constData(), fileSize);
    qDebug() << pdu->caData;
    qDebug() << (char*)pdu->caMsg;
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
    // 1s后上传文件，防止粘包
    m_pTimer->start(1000);
}

void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    // 如果打开文件失败
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }
    // 有人做过实验，4096传输数据时效率最高
    char *pBuffer = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        // 读取文件中的数据
        ret = file.read(pBuffer, 4096);
        // 如果读取到了文件中的数据，则发送给服务器
        if(ret > 0 && ret <= 4096)
        {
            TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);
        }
        // 如果文件读取结束了，就结束循环
        else if(ret == 0)
        {
            break;
        }
        // 如果ret < 0 或者 ret > 4096,则代表读取错误了
        else
        {
            QMessageBox::warning(this, "上传文件", "上传文件失败：读取文件内容失败");
            break;
        }
    }
    delete []pBuffer;
    pBuffer = NULL;
}

void Book::delFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
        return ;
    }
    QString strDelName = pItem->text();
    QByteArray utf8CurPath = strCurPath.toUtf8();
    PDU *pdu = mkPDU(utf8CurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
    strncpy(pdu->caData, strDelName.toUtf8().constData(), 32);
    memcpy((char*)pdu->caMsg, utf8CurPath.constData(), utf8CurPath.size() + 1);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::downloadFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件");
        return ;
    }
    QString strDownloadName = pItem->text();
    QString strSaveFilePath = QFileDialog::getSaveFileName(this, "保存文件", strDownloadName);
    if(strSaveFilePath.isEmpty())
    {
        QMessageBox::warning(this, "下载文件", "请指定要保存的位置");
        m_strSaveFilePath.clear();
        return ;
    }
    m_strSaveFilePath = strSaveFilePath;
    QByteArray utf8CurPath = strCurPath.toUtf8();
    PDU *pdu = mkPDU(utf8CurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    strncpy(pdu->caData, strDownloadName.toUtf8().constData(), 32);
    memcpy((char*)pdu->caMsg, utf8CurPath.constData(), utf8CurPath.size() + 1);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::shareFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "分享文件", "请选择要分享的文件");
        return ;
    }
    m_shareFileName = pItem->text();
    Friend *pFriend = OpeWidget::getInstance().getFriend();
    pFriend->flushFriend();// 异步刷新好友列表，返回后会通过 updateFriendList 填充分享窗口
    if(ShareFile::getInstance().isHidden())
    {
        ShareFile::getInstance().show();
    }
}

void Book::moveFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
        return ;
    }
    m_strMoveFileName = pItem->text();
    QString strCurPath = TcpClient::getInstance().curPath();
    m_strMoveFilePath = strCurPath + "/" +m_strMoveFileName;
    m_pMoveSelectDirPB->setEnabled(true);
}

void Book::selectDestDir()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "选择移动目录", "请选择要目标目录");
        return ;
    }
    QString strDestDir = pItem->text();
    QString strCurPath = TcpClient::getInstance().curPath();
    m_strSelectDestDirPath = strCurPath + "/" + strDestDir;
    m_pMoveSelectDirPB->setEnabled(false);

    QByteArray utf8MoveFilePath = m_strMoveFilePath.toUtf8();
    QByteArray utf8DestDirPath = m_strSelectDestDirPath.toUtf8();
    int srcLen = utf8MoveFilePath.size();
    int destLen = utf8DestDirPath.size();
    PDU *pdu = mkPDU(srcLen + destLen + 2);
    pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
    sprintf(pdu->caData, "%d %d %s", srcLen, destLen, m_strMoveFileName.toUtf8().constData());

    memcpy(pdu->caMsg, utf8MoveFilePath.constData(), srcLen + 1);
    memcpy((char*)(pdu->caMsg) + (srcLen + 1), utf8DestDirPath.constData(), destLen + 1);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}
