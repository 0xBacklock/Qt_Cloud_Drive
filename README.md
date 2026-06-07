# 云盘系统 (Qt_Cloud_Drive)

基于 Qt6 的 C/S 架构网盘系统，支持用户管理、好友聊天、文件云端存储与分享。

## 功能

- 用户注册 / 登录
- 好友管理（添加、删除、搜索、在线状态）
- 私聊 / 群聊
- 文件夹管理（创建、删除、重命名、进入、返回上级）
- 文件操作（上传、下载、删除、移动）
- 文件分享（发送给好友）
- 中英文完美支持（UTF-8 编码）

## 项目结构

```
QtDocuments/
├── TcpClient/           # 客户端
│   ├── main.cpp         # 入口，全局样式表
│   ├── tcpclient.h/cpp  # 登录窗口 + 消息路由
│   ├── opewidget.h/cpp  # 主界面（侧边栏 + 页面切换）
│   ├── friend.h/cpp     # 好友页（好友列表 + 群聊）
│   ├── book.h/cpp       # 文件页（文件浏览 + 操作）
│   ├── online.h/cpp     # 在线用户列表
│   ├── privatechat.h/cpp# 私聊窗口
│   ├── sharefile.h/cpp  # 文件分享对话框
│   ├── protocol.h/cpp   # 通信协议定义
│   ├── client.config    # 服务器 IP + 端口
│   └── *.ui             # Qt 界面文件
│
├── TcpServer-win/       # win服务端
│   ├── main.cpp         # 入口
│   ├── tcpserver.h/cpp  # 主窗口 + 配置加载
│   ├── mytcpserver.h/cpp# TCP 连接管理器（单例）
│   ├── mytcpsocket.h/cpp# 客户端请求处理器
│   ├── opedb.h/cpp      # SQLite 数据库操作（单例）
│   ├── protocol.h/cpp   # 通信协议定义（与客户端一致）
│   ├── cloud.db         # SQLite 数据库
│   ├── server.config    # 监听配置
│   └── tcpserver.ui     # 窗口界面
│
└── TcpServer-linux/     # linux服务端
    ├── main.cpp         # 入口
    ├── tcpserver.h/cpp  # 主窗口 + 配置加载
    ├── mytcpserver.h/cpp# TCP 连接管理器（单例）
    ├── mytcpsocket.h/cpp# 客户端请求处理器
    ├── opedb.h/cpp      # SQLite 数据库操作（单例）
    ├── protocol.h/cpp   # 通信协议定义（与客户端一致）
    ├── cloud.db         # SQLite 数据库
    ├── server.config    # 监听配置
    ├── run.sh           # Linux 服务管理脚本
    └── tcpserver.ui     # 窗口界面
```

## 技术栈

| 层 | 技术 |
|---|---|
| UI | Qt6 Widgets + QSS 样式表 |
| 网络 | Qt6 Network (QTcpServer / QTcpSocket) |
| 数据库 | SQLite (Qt6 SQL 模块) |
| 协议 | 自定义二进制 PDU 协议 |
| 编码 | 全链路 UTF-8 |
| 构建 | qmake + MinGW (Windows) / g++ (Linux) |

## 通信协议

所有数据通过 `PDU`（Protocol Data Unit）结构体传输：

```
┌────────────┬───────────┬──────────────┬───────────┬──────────┐
│ uiPDULen   │ uiMsgType │ caData[64]   │ uiMsgLen  │ caMsg[]  │
│ 4 bytes    │ 4 bytes   │ 64 bytes     │ 4 bytes   │ 可变长度  │
└────────────┴───────────┴──────────────┴───────────┴──────────┘
```

- `uiPDULen`: PDU 总大小
- `uiMsgType`: 消息类型（~50 种，Request/Response 配对）
- `caData[64]`: 固定附加数据（通常前 32 字节和后 32 字节分别存放两个参数）
- `uiMsgLen`: 可变消息体长度
- `caMsg[]`: 可变消息体（文件名、路径、聊天内容等）

## 构建

### Windows

Qt Creator 打开 `TcpClient/TcpClient.pro` 和 `TcpServer/TcpServer.pro`，选择 MinGW 64-bit 套件编译。

或者在命令行：

```powershell
cd TcpServer
qmake TcpServer.pro
mingw32-make

cd ..\TcpClient
qmake TcpClient.pro
mingw32-make
```

### Linux（仅服务端）

```bash
# 安装依赖
sudo apt install qt6-base-dev libqt6sql6-sqlite build-essential

# 编译
cd TcpServer
qmake6 TcpServer.pro
make -j$(nproc)
```

## 部署

### 服务端 (Linux)

```bash
# 启动
cd /path/to/TcpServer
./run.sh start

# 停止
./run.sh stop

# 查看状态
./run.sh status

# 实时日志
./run.sh logs
```

默认监听 `0.0.0.0:8888`，可在可执行文件同目录的 `server.config` 中自定义：

```
0.0.0.0
8888
```

### 客户端 (Windows)

修改 `client.config` 为服务器 IP，然后启动 `TcpClient.exe`：

```
你的服务器IP
8888
```

## 数据库

`cloud.db` 包含两张表：

```sql
-- 用户表
CREATE TABLE usrInfo (
    id     INTEGER PRIMARY KEY AUTOINCREMENT,
    name   TEXT UNIQUE,
    pwd    TEXT,
    online INTEGER DEFAULT 0
);

-- 好友关系表（双向存储）
CREATE TABLE friend (
    id       INTEGER,
    friendId INTEGER
);
```

管理命令：

```bash
# 清空所有数据
sqlite3 cloud.db "DELETE FROM usrInfo; DELETE FROM friend;"

# 查看用户
sqlite3 cloud.db "SELECT * FROM usrInfo;"
```
