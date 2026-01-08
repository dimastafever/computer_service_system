# Docker Deployment TODO

## Phase 1: Pre-deployment Check
- [x] Review Dockerfile configuration
- [x] Review docker-compose.yml configuration  
- [x] Verify Grafana provisioning config exists
- [x] Verify database init.sql exists
- [x] Verify monitoring configs (prometheus.yml, promtail-config.yaml)

## Phase 2: Build Images
- [ ] Run docker-compose build
- [ ] Verify libpqxx builds successfully
- [ ] Verify application compiles
- [ ] Verify final image size

## Phase 3: Start Services
- [ ] Start PostgreSQL
- [ ] Start Service System
- [ ] Start Prometheus
- [ ] Start Loki
- [ ] Start Promtail
- [ ] Start Grafana

## Phase 4: Verify System
- [ ] Check all containers are running (docker-compose ps)
- [ ] Verify database health check passes
- [ ] Verify application health check passes
- [ ] Test /metrics endpoint
- [ ] Test /api/test-db endpoint
- [ ] Verify Prometheus targets are up
- [ ] Verify Grafana can connect to Prometheus
- [ ] Verify Grafana can connect to Loki

## Phase 5: Integration Tests
- [ ] Test full API workflow
- [ ] Verify logs appear in Grafana
- [ ] Verify metrics appear in Grafana
- [ ] Test authentication endpoints

## Commands Reference
```bash
# Build
docker-compose build

# Start
docker-compose up -d

# Stop
docker-compose down

# Stop with volumes
docker-compose down -v

# View logs
docker-compose logs -f

# View specific service logs
docker-compose logs -f service-system
docker-compose logs -f prometheus
docker-compose logs -f grafana
```

