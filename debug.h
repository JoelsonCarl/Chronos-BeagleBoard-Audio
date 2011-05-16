/* Debugging and Error Output */
#ifdef _DEBUG
    #define DBG(fmt, args...) fprintf(stderr, "Debug: " fmt, ## args)
#else
    #define DBG(fmt, args...)
#endif

#define ERR(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)
