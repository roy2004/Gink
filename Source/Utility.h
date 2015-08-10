#pragma once


#define LENGTH_OF(ARRAY) \
    (sizeof (ARRAY) / sizeof *(ARRAY))

#define STRINGIZE(TEXT) \
    __STRINGIZE(TEXT)

#define __STRINGIZE(TEXT) \
    #TEXT
