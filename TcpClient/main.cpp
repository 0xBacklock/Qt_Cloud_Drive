#include "tcpclient.h"
#include <QApplication>
#include"sharefile.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyleSheet(R"(
        /* 全局字体 */
        * {
            font-family: "Microsoft YaHei", "Segoe UI", "PingFang SC", sans-serif;
            font-size: 14px;
        }

        /* 输入框 */
        QLineEdit {
            border: 1px solid #dcdfe6;
            border-radius: 6px;
            padding: 8px 12px;
            background: white;
            font-size: 14px;
        }
        QLineEdit:focus {
            border-color: #409eff;
        }

        /* 主要按钮 */
        QPushButton {
            background-color: #409eff;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #66b1ff;
        }
        QPushButton:pressed {
            background-color: #3a8ee6;
        }

        /* 列表 */
        QListWidget {
            border: 1px solid #ebeef5;
            border-radius: 6px;
            background: white;
            padding: 4px;
            outline: none;
        }
        QListWidget::item {
            padding: 6px 10px;
            border-radius: 4px;
        }
        QListWidget::item:hover {
            background: #f0f5ff;
        }
        QListWidget::item:selected {
            background: #d9ecff;
            color: #303133;
        }

        /* 文本编辑框 */
        QTextEdit {
            border: 1px solid #ebeef5;
            border-radius: 6px;
            background: #fafafa;
            padding: 8px;
        }

        /* 对话框 */
        QMessageBox {
            background: white;
        }

        /* 输入对话框 */
        QInputDialog QLineEdit {
            border: 1px solid #dcdfe6;
            border-radius: 4px;
            padding: 6px 10px;
        }
    )");

    TcpClient &w = TcpClient::getInstance();
    w.setWindowTitle("云盘客户端");
    w.show();
    return a.exec();
}
