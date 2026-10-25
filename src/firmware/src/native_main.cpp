// No-op main so `pio run -e native` links (a compile+link sanity check of the
// portable modules). The real host unit test is test/host_test.cpp, which is run
// via test/run_host_test.sh and supplies its own main().
#ifndef ARDUINO
int main() { return 0; }
#endif
