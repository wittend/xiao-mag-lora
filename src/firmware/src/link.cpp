// Portable link logic (seq allocation, gap/loss estimation) is header-only in
// link.hpp. This translation unit exists so the PlatformIO / g++ source lists
// stay uniform; it currently compiles to nothing beyond the header.
#include "link.hpp"
