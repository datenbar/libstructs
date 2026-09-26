/*
 * Used only by the freestanding-check target in CMakeLists.txt. A real
 * kernel provides its own kernel_panic() this stub exists purely so the
 * check can fully link generated code built with -DLIBSTRUCTS_KERNEL_BUILD
 * and confirm nothing else pulls in libc.
 */
void kernel_panic(const char* msg) {
    (void)msg;
    for (;;) {
    }
}
