#!/bin/bash
# Скрипт для публикации образа в Docker Hub
# Repository: dmitriier/dev_ops_pr

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Docker Hub Push Script ===${NC}"
echo -e "${GREEN}Repository: dmitriier/dev_ops_pr${NC}"

# Проверка аргументов (опционально - использует dmitriier по умолчанию)
USERNAME="${1:-dmitriier}"
IMAGE_NAME="${USERNAME}/dev_ops_pr"
COMMIT_HASH=$(git rev-parse --short HEAD 2>/dev/null || echo "local")

echo -e "${GREEN}Имя образа: ${IMAGE_NAME}${NC}"
echo -e "${GREEN}Commit hash: ${COMMIT_HASH}${NC}"

# Авторизация
echo -e "${YELLOW}Введите ваш Docker Hub пароль или Access Token:${NC}"
docker login -u "$USERNAME"

# Сборка образа
echo -e "${GREEN}Сборка образа...${NC}"
docker build -t dev_ops_pr:latest .

# Тегирование
echo -e "${GREEN}Тегирование образа...${NC}"
docker tag dev_ops_pr:latest "${IMAGE_NAME}:latest"
docker tag dev_ops_pr:latest "${IMAGE_NAME}:${COMMIT_HASH}"
docker tag dev_ops_pr:latest "${IMAGE_NAME}:$(date +%Y%m%d)"

# Отправка
echo -e "${GREEN}Отправка в Docker Hub...${NC}"
docker push "${IMAGE_NAME}:latest"
docker push "${IMAGE_NAME}:${COMMIT_HASH}"
docker push "${IMAGE_NAME}:$(date +%Y%m%d)"

echo -e "${GREEN}=== Готово! ===${NC}"
echo -e "${GREEN}Образ ${IMAGE_NAME} успешно загружен в Docker Hub${NC}"
echo -e "Теги: latest, ${COMMIT_HASH}, $(date +%Y%m%d)"

echo -e "${YELLOW}Для запуска образа используйте:${NC}"
echo -e "${YELLOW}  docker pull ${IMAGE_NAME}:latest${NC}"
echo -e "${YELLOW}  docker run -p 8080:8080 ${IMAGE_NAME}:latest${NC}"

