#!/bin/bash

# Скрипт установки и настройки мониторинга для Service Management System
# Этот скрипт устанавливает и настраивает Promtail для сбора логов

set -e

echo "==================================="
echo "Настройка мониторинга"
echo "==================================="

# Пути
PROJECT_DIR="/home/adminstd/gitpr"
CONFIG_FILE="$PROJECT_DIR/promtail-local-config.yaml"
SERVICE_FILE="$PROJECT_DIR/promtail.service"

# Проверяем установлен ли Promtail
if ! command -v promtail &> /dev/null; then
    echo "[1/5] Promtail не найден. Скачивание и установка Promtail..."
    
    # Определяем архитектуру
    ARCH=$(uname -m)
    if [ "$ARCH" = "x86_64" ]; then
        PROMTAIL_URL="https://github.com/grafana/loki/releases/download/v2.9.2/promtail-linux-amd64"
    elif [ "$ARCH" = "aarch64" ]; then
        PROMTAIL_URL="https://github.com/grafana/loki/releases/download/v2.9.2/promtail-linux-arm64"
    else
        PROMTAIL_URL="https://github.com/grafana/loki/releases/download/v2.9.2/promtail-linux-amd64"
    fi
    
    # Скачиваем Promtail
    sudo wget -O /usr/local/bin/promtail "$PROMTAIL_URL"
    sudo chmod +x /usr/local/bin/promtail
    echo "Promtail установлен в /usr/local/bin/promtail"
else
    echo "[1/5] Promtail уже установлен: $(which promtail)"
fi

# Создаем директорию для логов
echo "[2/5] Создание директории для логов..."
mkdir -p "$PROJECT_DIR/logs"
touch "$PROJECT_DIR/logs/webserver.log"

# Проверяем конфигурацию Promtail
echo "[3/5] Проверка конфигурации Promtail..."
if [ -f "$CONFIG_FILE" ]; then
    echo "Конфигурация Promtail найдена: $CONFIG_FILE"
    
    # Обновляем путь к файлу логов в конфигурации
    sed -i "s|/home/adminstd/gitpr/logs/webserver.log|$PROJECT_DIR/logs/webserver.log|g" "$CONFIG_FILE"
    echo "Путь к логам обновлен в конфигурации"
else
    echo "Ошибка: Конфигурационный файл Promtail не найден: $CONFIG_FILE"
    exit 1
fi

# Копируем service файл
echo "[4/5] Установка systemd service..."
if [ -f "$SERVICE_FILE" ]; then
    sudo cp "$SERVICE_FILE" /etc/systemd/system/promtail.service
    sudo chmod 644 /etc/systemd/system/promtail.service
    echo "Service файл скопирован в /etc/systemd/system/promtail.service"
else
    echo "Ошибка: Service файл не найден: $SERVICE_FILE"
    exit 1
fi

# Перезагружаем systemd и запускаем сервисы
echo "[5/5] Запуск сервисов..."
sudo systemctl daemon-reload
sudo systemctl enable promtail.service
sudo systemctl start promtail.service

echo ""
echo "==================================="
echo "Мониторинг настроен!"
echo "==================================="
echo ""
echo "Состояние сервисов:"
echo "- Promtail: $(systemctl is-active promtail.service 2>/dev/null || echo 'not running')"
echo ""
echo "Логи приложения: $PROJECT_DIR/logs/webserver.log"
echo ""
echo "После запуска приложения выполните:"
echo "  1. Соберите проект: cd $PROJECT_DIR/build && cmake .. && make"
echo "  2. Запустите приложение: ./service_system"
echo ""
echo "Логи будут автоматически собираться Promtail и отправляться в Loki."

