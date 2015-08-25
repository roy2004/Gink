#pragma once


#define GINK_LENGTH_OF(array) \
    (sizeof (array) / sizeof *(array))

#define GINK_STRINGIZE(text) \
    __GINK_STRINGIZE(text)

#define __GINK_STRINGIZE(text) \
    #text
