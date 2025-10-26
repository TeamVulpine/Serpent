#pragma once

#if defined(SERPENT_SHARED)
    #if defined(_WIN32)
        #if defined(SERPENT_EXPORTS)
            #define SERPENT_API __declspec(dllexport)
        #else
            #define SERPENT_API __declspec(dllimport)
        #endif
    #else
        #define SERPENT_API __attribute__((visibility("default")))
    #endif
#else
    #define SERPENT_API
#endif
