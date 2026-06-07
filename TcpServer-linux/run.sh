#!/bin/bash

SERVER_DIR="$(cd "$(dirname "$0")" && pwd)"
SERVER_BIN="$SERVER_DIR/TcpServer"
LOG_FILE="$SERVER_DIR/server.log"
PID_FILE="$SERVER_DIR/server.pid"

start() {
    if [ -f "$PID_FILE" ] && kill -0 $(cat "$PID_FILE") 2>/dev/null; then
        echo "服务已在运行中 (PID: $(cat $PID_FILE))"
        return 1
    fi

    if [ ! -f "$SERVER_BIN" ]; then
        echo "未找到 TcpServer，请先执行 make 编译"
        return 1
    fi

    cd "$SERVER_DIR"
    QT_QPA_PLATFORM=offscreen nohup "$SERVER_BIN" > "$LOG_FILE" 2>&1 &
    echo $! > "$PID_FILE"
    echo "服务已启动 (PID: $!)"
    echo "日志文件: $LOG_FILE"
}

stop() {
    if [ ! -f "$PID_FILE" ]; then
        echo "服务未运行（无 PID 文件）"
        return 1
    fi

    PID=$(cat "$PID_FILE")
    if kill -0 "$PID" 2>/dev/null; then
        kill "$PID"
        rm -f "$PID_FILE"
        echo "服务已停止 (PID: $PID)"
    else
        echo "PID $PID 不存在，清理 PID 文件"
        rm -f "$PID_FILE"
    fi
}

restart() {
    stop
    sleep 1
    start
}

status() {
    if [ -f "$PID_FILE" ] && kill -0 $(cat "$PID_FILE") 2>/dev/null; then
        echo "服务运行中 (PID: $(cat $PID_FILE))"
        echo "监听端口:"
        ss -tlnp 2>/dev/null | grep 8888 || netstat -tlnp 2>/dev/null | grep 8888
    else
        echo "服务未运行"
    fi
}

logs() {
    tail -f "$LOG_FILE"
}

case "${1:-start}" in
    start)   start ;;
    stop)    stop ;;
    restart) restart ;;
    status)  status ;;
    logs)    logs ;;
    *)
        echo "用法: $0 {start|stop|restart|status|logs}"
        exit 1
        ;;
esac
