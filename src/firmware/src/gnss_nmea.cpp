#include "gnss_nmea.hpp"
#include <cstdlib>
#include <cstring>

namespace xlora::gnss {

namespace {
// Days since 1970-01-01 for a civil date (Howard Hinnant's algorithm).
std::int64_t days_from_civil(int y, int m, int d) {
    y -= (m <= 2);
    const int era = (y >= 0 ? y : y - 399) / 400;
    const int yoe = y - era * 400;                              // [0, 399]
    const int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;  // [0, 365]
    const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;     // [0, 146096]
    return static_cast<std::int64_t>(era) * 146097 + doe - 719468;
}
// Parse exactly two ASCII digits (returns -1 if either is not a digit).
int two_digits(const char* p) {
    if (p[0] < '0' || p[0] > '9' || p[1] < '0' || p[1] > '9') return -1;
    return (p[0] - '0') * 10 + (p[1] - '0');
}
}  // namespace

GpsFix parse_rmc(const char* s) {
    GpsFix out;
    if (!s) return out;

    const char* dollar = std::strchr(s, '$');
    if (!dollar) return out;
    const char* star = std::strchr(dollar, '*');
    if (!star) return out;

    // NMEA checksum: XOR of every byte between '$' and '*'.
    std::uint8_t calc = 0;
    for (const char* p = dollar + 1; p < star; ++p) calc ^= static_cast<std::uint8_t>(*p);
    const std::uint8_t got = static_cast<std::uint8_t>(
        std::strtol(star + 1, nullptr, 16) & 0xFF);
    if (calc != got) return out;

    // Copy the body (between '$' and '*') and split on ','.
    const std::size_t body_len = static_cast<std::size_t>(star - dollar - 1);
    if (body_len == 0 || body_len >= 255) return out;
    char body[256];
    std::memcpy(body, dollar + 1, body_len);
    body[body_len] = '\0';

    const char* field[16];
    int nf = 0;
    field[nf++] = body;
    for (char* p = body; *p != '\0' && nf < 16; ++p) {
        if (*p == ',') {
            *p = '\0';
            field[nf++] = p + 1;
        }
    }
    if (nf < 10) return out;

    // $GPRMC: [1]=UTC time hhmmss.s, [2]=status A/V, [9]=date ddmmyy.
    if (std::strlen(field[2]) < 1 || field[2][0] != 'A') return out;

    const char* t = field[1];
    if (std::strlen(t) < 6) return out;
    const int hh = two_digits(t + 0);
    const int mm = two_digits(t + 2);
    const int ss = two_digits(t + 4);
    if (hh < 0 || mm < 0 || ss < 0) return out;

    const char* d = field[9];
    if (std::strlen(d) < 6) return out;
    const int dd = two_digits(d + 0);
    const int mo = two_digits(d + 2);
    const int yy = two_digits(d + 4);
    if (dd < 0 || mo < 0 || yy < 0) return out;

    if (hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59) return out;
    if (mo < 1 || mo > 12 || dd < 1 || dd > 31) return out;

    const int year = 2000 + yy;
    const std::int64_t days = days_from_civil(year, mo, dd);
    out.epoch = static_cast<std::uint32_t>(days * 86400 + hh * 3600 + mm * 60 + ss);
    out.valid = true;
    return out;
}

}  // namespace xlora::gnss
