CREATE TABLE vessels (
    mmsi BIGINT PRIMARY KEY,
    imo BIGINT,
    name VARCHAR(255),
    callsign VARCHAR(100),
    vessel_type INTEGER,
    length_m NUMERIC(10, 2),
    width_m NUMERIC(10, 2),
    destination VARCHAR(255),
    eta TIMESTAMP,
    draught_m NUMERIC(10, 2),
    updated_at TIMESTAMP NOT NULL DEFAULT NOW()
);

CREATE TABLE position_reports(
    id BIGSERIAL PRIMARY KEY,
    mmsi BIGINT NOT NULL REFERENCES vessels(mmsi) ON DELETE CASCADE,
    latitude DOUBLE PRECISION NOT NULL,
    longitude DOUBLE PRECISION NOT NULL,
    <!-- Speed over ground in knots -->
    sog_knots NUMERIC(8, 3) NOT NULL,
    <!-- Course over ground in degrees -->
    cog_degrees NUMERIC(8, 3) NOT NULL,
    true_heading INTEGER NOT NULL,
    navigational_status INTEGER,
    ais_timestamp TIMESTAMP NOT NULL,
    received_at TIMESTAMP NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_position_reports_mmsi_time ON position_reports(mmsi, ais_timestamp DESC);
CREATE INDEX id_position_reports_time ON position_reports(ais_timestamp DESC); 