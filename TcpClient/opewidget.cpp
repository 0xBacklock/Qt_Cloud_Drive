#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("云盘");
    resize(900, 600);

    m_pFriend = new Friend;
    m_pBook = new Book;
    m_pSW = new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    m_pListW = new QListWidget(this);
    m_pListW->addItem("好友");
    m_pListW->addItem("云盘");
    m_pListW->setFixedWidth(120);
    m_pListW->setCurrentRow(0);
    m_pListW->setStyleSheet(
        "QListWidget { background: #f5f7fa; border: none; border-right: 1px solid #e4e7ed; font-size: 14px; }"
        "QListWidget::item { padding: 14px 16px; border: none; }"
        "QListWidget::item:hover { background: #e8edf3; }"
        "QListWidget::item:selected { background: #d9ecff; color: #409eff; border-right: 3px solid #409eff; }");

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->setContentsMargins(0, 0, 0, 0);
    pMain->setSpacing(0);
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
