include(CheckCSourceCompiles)

# Check if NEON is supported by compiling a test program
check_c_source_compiles("
    #include <arm_neon.h>
    int main() {
        float32x4_t a = vdupq_n_f32(0.0f);
        return 0;
    }
" SUPPORT_NEON)

if(SUPPORT_NEON)
    message(STATUS "NEON support enabled")
else()
    message(STATUS "NEON not supported")
endif()
