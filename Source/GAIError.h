#pragma once


#include <exception>
#include <string>

#include "Utility.h"


#define GAI_ERROR(ERROR_CODE, STRING) \
    GAIError(__FILE__ ":" STRINGIZE(__LINE__), ERROR_CODE, STRING)


class GAIError final: public std::exception
{
public:
    inline GAIError(const GAIError &);
    inline ~GAIError() override;
    inline void operator=(const GAIError &);

    inline int getErrorCode() const noexcept;
    inline const char *what() const noexcept override;

    explicit GAIError(const char *, int, const char *);

private:
    int errorCode_;
    std::string description_;
};


GAIError::GAIError(const GAIError &other)
    : std::exception(other), errorCode_(other.errorCode_), description_(other.description_)
{
}


GAIError::~GAIError()
{
}


void
GAIError::operator=(const GAIError &other)
{
    if (&other == this) {
        return;
    }

    std::exception::operator=(other);
    errorCode_ = other.errorCode_;
    description_ = other.description_;
}


int
GAIError::getErrorCode() const noexcept
{
    return errorCode_;
}


const char *
GAIError::what() const noexcept
{
    return description_.c_str();
}
