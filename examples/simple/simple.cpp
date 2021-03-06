/*
  Copyright (c) 2010-2011, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.


   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/

#include <stdio.h>
#include <stdlib.h>
#include "../cpuid.h"

// Include the header file that the ispc compiler generates
#include "simple_ispc.h"
using namespace ispc;

// Make sure that the vector ISA used during compilation is supported by
// the processor.  The ISPC_TARGET_* macro is set in the ispc-generated
// header file that we include above.
static void
ensureTargetISAIsSupported() {
#if defined(ISPC_TARGET_SSE2)
    bool isaSupported = CPUSupportsSSE2();
    const char *target = "SSE2";
#elif defined(ISPC_TARGET_SSE4)
    bool isaSupported = CPUSupportsSSE4();
    const char *target = "SSE4";
#elif defined(ISPC_TARGET_AVX)
    bool isaSupported = CPUSupportsAVX();
    const char *target = "AVX";
#else
#error "Unknown ISPC_TARGET_* value"
#endif
    if (!isaSupported) {
        fprintf(stderr, "***\n*** Error: the ispc-compiled code uses the %s instruction "
                "set, which isn't\n***        supported by this computer's CPU!\n", target);
        fprintf(stderr, "***\n***        Please modify the "
#ifdef _MSC_VER
                "MSVC project file "
#else
                "Makefile "
#endif
                "to select another target (e.g. sse2)\n***\n");
        exit(1);
    }
}


int main() {
    ensureTargetISAIsSupported();

    float vin[16], vout[16];

    // Initialize input buffer
    for (int i = 0; i < 16; ++i)
        vin[i] = (float)i;

    // Call simple() function from simple.ispc file
    simple(vin, vout, 16);

    // Print results
    for (int i = 0; i < 16; ++i)
        printf("%d: simple(%f) = %f\n", i, vin[i], vout[i]);

    return 0;
}
