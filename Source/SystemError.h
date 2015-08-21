#pragma once


#include <exception>
#include <string>

#include "Utility.h"


#define SYSTEM_ERROR(ERROR_NUMBER, STRING) \
    SystemError(__FILE__ ":" STRINGIZE(__LINE__), ERROR_NUMBER, STRING)


class SystemError final: public std::exception
{
    SystemError(const SystemError &) = delete;
    void operator=(const SystemError &) = delete;

public:
    inline SystemError(SystemError &&);
    inline ~SystemError() override;

    inline int getErrorNumber() const noexcept;
    inline const char *what() const noexcept override;

    explicit SystemError(const char *, int, const char *);

private:
    int errorNumber_;
    std::string description_;
};


SystemError::SystemError(SystemError &&other)
    : std::exception(std::move(other)), errorNumber_(other.errorNumber_)
      , description_(std::move(other.description_))
{
}


SystemError::~SystemError()
{
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
