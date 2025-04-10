include(CheckCSourceCompiles)

# Check if NEON is supported by compiling a test program
check_c_source_compiles("
    #include <arm_neon.h>
    int main() {
        float32x4_t a = vdupq_n_f32(0.0f);
        return 0;
    }
" SUPPORT_NEON)

# Alternatively, check for ARM architecture and NEON macros if the above fails
if(NOT SUPPORT_NEON)
    check_c_source_compiles("
        #if defined(__ARM_NEON__) || (defined(__ARM_NEON) && defined(__aarch64__))
        int main() { return 0; }
        #else
        #error NEON not available
        #endif
    " SUPPORT_NEON)
endif()

if(SUPPORT_NEON)
    message(STATUS "NEON support enabled")
else()
    message(STATUS "NEON not supported")
endif()
