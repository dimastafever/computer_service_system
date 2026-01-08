#!/bin/bash

# Test script to verify Prometheus metrics are being recorded
# Run this while the server is running

echo "=== Making test requests to the server ==="

echo ""
echo "1. Making request to root endpoint..."
curl -s http://localhost:8080/ > /dev/null
echo "   Done (status should be 200)"

echo ""
echo "2. Making request to /api/test-db..."
curl -s http://localhost:8080/api/test-db > /dev/null
echo "   Done (status should be 200)"

echo ""
echo "3. Making request to /api/devices (GET)..."
curl -s http://localhost:8080/api/devices > /dev/null
echo "   Done (status should be 200)"

echo ""
echo "4. Making request to /api/service-types..."
curl -s http://localhost:8080/api/service-types > /dev/null
echo "   Done (status should be 200)"

echo ""
echo "5. Making request to /api/service-history..."
curl -s http://localhost:8080/api/service-history > /dev/null
echo "   Done (status should be 200)"

echo ""
echo "6. Making request to /api/service-records..."
curl -s http://localhost:8080/api/service-records > /dev/null
echo "   Done (status should be 200)"

echo ""
echo "7. Making logout request..."
curl -s -X POST http://localhost:8080/api/logout > /dev/null
echo "   Done (status should be 200)"

echo ""
echo "8. Making login request (POST - success with admin/admin123)..."
curl -s -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}' > /dev/null
echo "   Done (status should be 200)"

echo ""
echo "9. Making login request (POST - fail with wrong credentials)..."
curl -s -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{"username":"unknown","password":"wrong"}' > /dev/null
echo "   Done (status should be 401)"

echo ""
echo "10. Making request to /metrics to verify all metrics..."
echo ""
echo "=== Metrics output ==="
curl -s http://localhost:8080/metrics | grep -E "^http_requests_total|^auth_failures_total|^db_operations_total"

echo ""
echo "=== Test complete ==="
echo ""
echo "All requests should have been recorded with their respective status codes:"
echo "- 200 for successful requests"
echo "- 401 for failed login attempts"

