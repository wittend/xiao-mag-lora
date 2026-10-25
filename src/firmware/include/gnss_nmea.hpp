#pragma once
#include <cstdint>

namespace xlora::gnss {

// Result of parsing a GNSS sentence into a UTC clock value.
struct GpsFix {
    bool valid = false;      // true if the sentence parsed, checksum ok, mode A
    std::uint32_t epoch = 0; // UTC seconds since 1970-01-01
};

// Parse a single $GPRMC sentence (with or without trailing \r\n) into a UTC
// epoch. The L76K is used for clock discipline only (no position is kept);
// $GPRMC carries both the UTC time (field 1) and the date (field 9).
//
// Returns valid=false on: missing '$'/'*', NMEA checksum mismatch, too few
// fields, non-'A' status, or out-of-range time/date components.
GpsFix parse_rmc(const char* sentence);

}  // namespace xlora::gnss
