#pragma once

#include "domain/vessel.hpp"

#include <optional>

class VesselRepository {
public:
    virtual ~VesselRepository() = default;
    virtual void save(const Vessel& vessel) = 0;
    virtual std::optional<Vessel> findByMmsi(long long mmsi) = 0;
};