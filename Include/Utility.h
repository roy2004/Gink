#pragma once


#define GINK_STRINGIZE(text) \
    __GINK_STRINGIZE(text)

#define __GINK_STRINGIZE(text) \
    #text
