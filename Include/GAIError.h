#pragma once


#include <exception>
#include <string>

#include "Utility.h"


#define GINK_GAI_ERROR(errorCode, string) \
    Gink::GAIError(errorCode, __FILE__ ":" GINK_STRINGIZE(__LINE__) ": " string)


namespace Gink {

class GAIError final: public std::exception
{
    GAIError(const GAIError &) = delete;
    void operator=(const GAIError &) = delete;

public:
    inline GAIError(GAIError &&);
    inline ~GAIError() override;

    inline int getErrorCode() const noexcept;
    inline const char *what() const noexcept override;

    explicit GAIError(int, const char *);

private:
    int errorCode_;
    std::string description_;
};


GAIError::GAIError(GAIError &&other)
    : std::exception(std::move(other)), errorCode_(other.errorCode_)
      , description_(std::move(other.description_))
{
}


GAIError::~GAIError()
{
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

} // namespace Gink
