#pragma once


#include <exception>
#include <string>

#include "Utility.h"


#define SYSTEM_ERROR(ERROR_NUMBER, STRING) \
    SystemError(__FILE__ ":" STRINGIZE(__LINE__), ERROR_NUMBER, STRING)


class SystemError final: public std::exception
{
public:
    inline SystemError(const SystemError &);
    inline ~SystemError() override;
    inline void operator=(const SystemError &);

    inline int getErrorNumber() const noexcept;
    inline const char *what() const noexcept override;

    explicit SystemError(const char *, int, const char *);

private:
    int errorNumber_;
    std::string description_;
};


SystemError::SystemError(const SystemError &other)
    : std::exception(other), errorNumber_(other.errorNumber_), description_(other.description_)
{
}


SystemError::~SystemError()
{
}


void
SystemError::operator=(const SystemError &other)
{
    if (&other == this) {
        return;
    }

    std::exception::operator=(other);
    errorNumber_ = other.errorNumber_;
    description_ = other.description_;
}


int
SystemError::getErrorNumber() const noexcept
{
    return errorNumber_;
}


const char *
SystemError::what() const noexcept
{
    return description_.c_str();
}
