#!/bin/bash
# Скрипт для публикации образа в Docker Hub

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Docker Hub Push Script ===${NC}"

# Проверка аргументов
if [ -z "$1" ]; then
    echo -e "${YELLOW}Использование: $0 <dockerhub-username>${NC}"
    echo -e "${YELLOW}Пример: $0 myuser${NC}"
    exit 1
fi

USERNAME="$1"
IMAGE_NAME="${USERNAME}/service-system"
COMMIT_HASH=$(git rev-parse --short HEAD)

echo -e "${GREEN}Имя образа: ${IMAGE_NAME}${NC}"
echo -e "${GREEN}Commit hash: ${COMMIT_HASH}${NC}"

# Авторизация
echo -e "${YELLOW}Введите ваш Docker Hub пароль или Access Token:${NC}"
docker login -u "$USERNAME"

# Сборка образа
echo -e "${GREEN}Сборка образа...${NC}"
docker build -t service-system:latest .

# Тегирование
echo -e "${GREEN}Тегирование образа...${NC}"
docker tag service-system:latest "${IMAGE_NAME}:latest"
docker tag service-system:latest "${IMAGE_NAME}:${COMMIT_HASH}"
docker tag service-system:latest "${IMAGE_NAME}:$(date +%Y%m%d)"

# Отправка
echo -e "${GREEN}Отправка в Docker Hub...${NC}"
docker push "${IMAGE_NAME}:latest"
docker push "${IMAGE_NAME}:${COMMIT_HASH}"
docker push "${IMAGE_NAME}:$(date +%Y%m%d)"

echo -e "${GREEN}=== Готово! ===${NC}"
echo -e "${GREEN}Образ ${IMAGE_NAME} успешно загружен в Docker Hub${NC}"
echo -e "Теги: latest, ${COMMIT_HASH}, $(date +%Y%m%d)"

