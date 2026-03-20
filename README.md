# Maritime Traffic Monitoring

Maritime Traffic Monitoring is a small C++ backend for ingesting and querying vessel position reports. It exposes a REST API built with Crow, stores data in PostgreSQL through `libpqxx`, and is currently focused on position tracking rather than full AIS message processing.

## Current Scope

- Ingest vessel position reports
- Query the latest known position for a vessel by MMSI
- Query historical positions within a time range
- Expose a simple health check endpoint

The database schema currently contains:

- `vessels`
- `position_reports`

## Tech Stack

- C++20
- CMake + Ninja
- Crow
- PostgreSQL
- `libpqxx`
- vcpkg manifest dependencies

## Prerequisites

To build and run the project locally you need:

- A C++20-capable compiler
- CMake 3.31 or newer
- Ninja
- PostgreSQL running locally or remotely

This repository already includes a local vcpkg checkout under `.deps/vcpkg`, and the CMake presets are configured to use it.

## Configuration

The application reads its configuration from environment variables:

```env
APP_PORT=8080
APP_MULTITHREADED=true
DB_HOST=localhost
DB_PORT=5432
DB_NAME=maritime_traffic
DB_USER=monitor
DB_PASSWORD=monitor_password
DB_SSLMODE=disable
```

An `.env` file is present in the repository, but the application does not load it automatically. Export it in your shell before starting the binary.

## Local Development

1. Start PostgreSQL and create the target database.
2. Apply the initial schema:

```bash
psql -h localhost -U monitor -d maritime_traffic -f src/migration/001_init.sql
```

3. Export the application environment:

```bash
set -a
source .env
set +a
```

4. Configure and build:

```bash
cmake --preset debug
cmake --build --preset build-debug
```

5. Run the service:

```bash
./build/debug/maritime_traffic_monitoring
```

## API

### Health Check

```http
GET /health
```

Response:

```json
{
  "status": "ok"
}
```

### Ingest Position Report

```http
POST /api/v1/positions
Content-Type: application/json
```

Example request body:

```json
{
  "mmsi": 219000001,
  "latitude": 55.6761,
  "longitude": 12.5683,
  "sogKnots": 12.4,
  "cogDegrees": 86.2,
  "trueHeading": 85,
  "navStatus": 0,
  "aisTimestamp": "2026-03-20T08:30:00Z"
}
```

Required fields:

- `mmsi`
- `latitude`
- `longitude`
- `sogKnots`
- `cogDegrees`
- `trueHeading`
- `aisTimestamp`

### Get Latest Position

```http
GET /api/v1/positions/{mmsi}/latest
```

### Get Position History

```http
GET /api/v1/positions/{mmsi}/history?from=2026-03-20T00:00:00Z&to=2026-03-20T23:59:59Z
```

Timestamps are expected in UTC ISO 8601 format.

## Validation Rules

Incoming position reports are validated before persistence:

- `mmsi` must be greater than `0`
- latitude must be between `-90` and `90`
- longitude must be between `-180` and `180`
- `sogKnots` must be non-negative
- `cogDegrees` must be in the range `0 <= value < 360`
- `trueHeading` must be in the range `0 <= value < 360`
- `aisTimestamp` cannot be more than 5 minutes in the future

## Docker Notes

The repository includes a `docker-compose.yml` with PostgreSQL and app service definitions, but there is currently no `Dockerfile` in the project root. That means the database service is usable as-is, while the application service definition is not yet buildable without adding a Dockerfile.

## Upcoming Features

Planned next steps for the project include expanding beyond direct position ingestion into external AIS data integration. The main idea is to consume third-party AIS APIs, normalize the incoming payloads, and feed them into the existing storage and query flow.

Possible additions in this area include:

- Scheduled ingestion from external AIS providers
- Mapping provider-specific payloads into the internal position-report model
- Basic deduplication and validation of externally sourced vessel positions
- Support for mixing simulated traffic with live AIS data feeds

## Project Status

This is an early-stage backend. The current implementation covers the position-report flow, while several files in the repository suggest future expansion into vessel and AIS-oriented features. There are no automated tests checked in yet under `tests/`.
