#include <stdio.h>
extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include "cpp_patch_tap.h"
#include "tap.h"
#ifdef TAP_COMPILE
#include "cpp_patch_tap.c"
#include "tap.c"
#endif
#pragma GCC diagnostic pop
}
