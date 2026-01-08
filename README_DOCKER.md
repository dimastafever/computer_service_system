# Docker Deployment Guide

## Настройка DockerHub для CI/CD

### Шаг 1: Создайте DockerHub Token

1. Войдите в [Docker Hub](https://hub.docker.com)
2. Перейдите в Account Settings → Security → New Access Token
3. Скопируйте токен

### Шаг 2: Добавьте secrets в GitHub

1. Откройте репозиторий на GitHub
2. Перейдите в Settings → Secrets and variables → Actions
3. Добавьте следующие secrets:

| Secret Name | Value |
|-------------|-------|
| DOCKERHUB_USERNAME | Ваше имя пользователя Docker Hub |
| DOCKERHUB_TOKEN | Токен доступа из шага 1 |

### Шаг 3: Настройте IMAGE_NAME

В файле `.github/workflows/docker-publish.yml` замените:
```yaml
IMAGE_NAME: ${{ secrets.DOCKERHUB_USERNAME }}/service-system
```

на ваше реальное имя репозитория, например:
```yaml
IMAGE_NAME: myusername/service-system
```

## Локальная публикация образа

```bash
# Авторизуйтесь в Docker Hub
docker login -u $DOCKERHUB_USERNAME -p $DOCKERHUB_TOKEN

# Соберите образ
docker build -t service-system:latest .

# Тегируйте для Docker Hub
docker tag service-system:latest $DOCKERHUB_USERNAME/service-system:latest
docker tag service-system:latest $DOCKERHUB_USERNAME/service-system:$(git rev-parse --short HEAD)

# Отправьте в репозиторий
docker push $DOCKERHUB_USERNAME/service-system:latest
docker push $DOCKERHUB_USERNAME/service-system:$(git rev-parse --short HEAD)
```

## Запуск из Docker Hub

```bash
docker pull $DOCKERHUB_USERNAME/service-system:latest
docker run -p 8080:8080 -e DB_HOST=db -e DB_PORT=5432 $DOCKERHUB_USERNAME/service-system:latest
```

## CI/CD Pipeline

При пуше в `main`/`master` ветку:
1. Сборка образа с использованием GitHub Actions cache
2. Сканирование на уязвимости (Trivy)
3. Публикация в Docker Hub с тегами:
   - `latest` (для main/master)
   - `sha-{commit_hash}`
   - `pr-{pr_number}` (для pull requests)

