#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent) : QWidget(parent)
{
    m_pFriend = new Friend;
    m_pBook = new Book;
    m_pSW = new QStackedWidget;
    // 创建
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);
    //
    m_pListW = new QListWidget(this);
    m_pListW->addItem("好友");
    m_pListW->addItem("云盘");
    // 显示界面
    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_pSW);
    setLayout(pMain);
    // tab的切换
    connect(m_pListW, &QListWidget::currentRowChanged, m_pSW, &QStackedWidget::setCurrentIndex);
    // 切换到云盘页时自动刷新文件列表
    connect(m_pListW, &QListWidget::currentRowChanged, [this](int row){
        if(row == 1){
            m_pBook->flushDir();
        }
    });
}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;
}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pBook;
}
