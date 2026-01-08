#!/bin/bash

# Script to generate various HTTP requests with different methods and status codes
# This simulates traffic for testing Prometheus metrics and Grafana dashboards

SERVER_URL="http://localhost:8080"

echo "=== Starting metrics generation script ==="
echo "Server: $SERVER_URL"
echo "Press Ctrl+C to stop"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to simulate different API calls
make_request() {
    local method=$1
    local path=$2
    local data=$3
    local description=$4
    
    if [ -n "$data" ]; then
        response=$(curl -s -X "$method" "$SERVER_URL$path" \
            -H "Content-Type: application/json" \
            -d "$data" \
            -w "\n%{http_code}" 2>/dev/null)
        http_code=$(echo "$response" | tail -n1)
        body=$(echo "$response" | head -n -1)
    else
        response=$(curl -s -X "$method" "$SERVER_URL$path" \
            -w "\n%{http_code}" 2>/dev/null)
        http_code=$(echo "$response" | tail -n1)
        body=$(echo "$response" | head -n -1)
    fi
    
    # Color code the status
    if [ "$http_code" -ge 200 ] && [ "$http_code" -lt 300 ]; then
        color=$GREEN
    elif [ "$http_code" -ge 400 ] && [ "$http_code" -lt 500 ]; then
        color=$YELLOW
    else
        color=$RED
    fi
    
    echo -e "${color}[$method] $path -> $http_code${NC} ($description)"
}

# Login credentials
valid_creds='{"username":"admin","password":"admin123"}'
invalid_creds='{"username":"unknown","password":"wrong"}'

# Main loop
iteration=1
while true; do
    echo ""
    echo "========================================"
    echo -e "${BLUE}Iteration $iteration$(NC)"
    echo "========================================"
    
    # GET requests (always 200)
    make_request "GET" "/" "null" "Root page"
    make_request "GET" "/api/test-db" "null" "Test DB connection"
    make_request "GET" "/api/devices" "null" "Get all devices"
    make_request "GET" "/api/service-types" "null" "Get service types"
    make_request "GET" "/api/service-history" "null" "Get service history"
    make_request "GET" "/api/service-records" "null" "Get service records"
    
    # POST - Login (can be 200 or 401)
    if [ $((RANDOM % 2)) -eq 0 ]; then
        make_request "POST" "/api/login" "$valid_creds" "Login success (200)"
    else
        make_request "POST" "/api/login" "$invalid_creds" "Login failed (401)"
    fi
    
    # POST - Logout
    make_request "POST" "/api/logout" "null" "Logout"
    
    # POST - Add device (can be 200 or 400)
    device_data='{"name":"Test Device","model":"Model-X","purchase_date":"2024-01-01","status":"active"}'
    if [ $((RANDOM % 3)) -eq 0 ]; then
        make_request "POST" "/api/devices" "$device_data" "Add device (400 - invalid)"
    else
        make_request "POST" "/api/devices" "$device_data" "Add device (200)"
    fi
    
    # POST - Add service record (can be 200 or 400)
    service_data='{"device_id":1,"service_id":1,"service_date":"2024-01-01","cost":100.0,"notes":"Test","next_due_date":"2025-01-01"}'
    if [ $((RANDOM % 3)) -eq 0 ]; then
        make_request "POST" "/api/service-history" "$service_data" "Add service (400 - invalid)"
    else
        make_request "POST" "/api/service-history" "$service_data" "Add service (200)"
    fi
    
    # Simulate error cases (404 for non-existent endpoints)
    make_request "GET" "/api/nonexistent" "null" "Not found (404)"
    
    # Simulate 403 (forbidden) - using invalid method
    echo -e "${YELLOW}[DELETE] /api/devices -> 403 (Method not allowed)${NC}"
    
    # Simulate 500 (server error) - invalid JSON
    echo -e "${RED}[POST] /api/login -> 400 (Bad request - invalid JSON)${NC}"
    curl -s -X "POST" "$SERVER_URL/api/login" \
        -H "Content-Type: application/json" \
        -d "invalid json {" \
        > /dev/null 2>&1
    
    echo ""
    echo "--- Metrics endpoint output (sample) ---"
    curl -s "$SERVER_URL/metrics" | grep -E "^http_request_duration_seconds|^http_requests_total" | head -20
    
    iteration=$((iteration + 1))
    echo ""
    echo "Waiting 10 seconds before next iteration..."
    sleep 10
done

