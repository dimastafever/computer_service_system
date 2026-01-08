# Deployment Plan

## Steps to Deploy Service Management System

### 1. Fix libpqxx Headers (for system libpqxx compatibility)
- ✅ Dockerfile updated to use system libpqxx from Debian apt
- ✅ CMakeLists.txt updated to use pkg-config for libpqxx discovery

### 2. Build Docker Image
```bash
cd /home/adminstd/gitpr
docker build -t service-system .
```

### 3. Deploy with Docker Compose
```bash
docker-compose up -d
```

### 4. Verify Services
```bash
# Check all containers are running
docker-compose ps

# Check application health
curl http://localhost:8080/health

# Check Prometheus
curl http://localhost:9090

# Check Grafana (admin/admin)
http://localhost:3000
```

### 5. View Logs
```bash
docker-compose logs -f service-system
```

## Expected Services
- **db**: PostgreSQL database on port 5432
- **service-system**: Application on port 8080
- **prometheus**: Metrics collection on port 9090
- **loki**: Log storage on port 3100
- **promtail**: Log collector
- **grafana**: Visualization on port 3000

## Troubleshooting libpqxx Issues
If you see errors like:
- `undefined reference to pqxx::*`
- `cannot find -lpqxx`

Make sure:
1. libpqxx-dev is installed in the builder stage
2. CMakeLists.txt uses `${PQXX_LIBRARIES}` and `${PQXX_LINK_LIBRARIES}`
3. pkg-config finds pqxx correctly

## Common Commands
```bash
# Stop all services
docker-compose down

# Stop and remove volumes
docker-compose down -v

# Rebuild after changes
docker-compose build --no-cache
docker-compose up -d
```

