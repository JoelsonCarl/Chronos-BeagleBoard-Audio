/*
 *   debug.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2005
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <stdio.h>

// Enables or disables debug output
#ifdef _DEBUG
    #define DBG(fmt, args...) fprintf(stderr, "Debug: " fmt, ## args)
#else
    #define DBG(fmt, args...)
#endif

#define ERR(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)
