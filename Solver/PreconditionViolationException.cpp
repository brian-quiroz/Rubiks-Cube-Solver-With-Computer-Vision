#include "PreconditionViolationException.h"

PreconditionViolationException::PreconditionViolationException(std::string error_message) : std::runtime_error(error_message) {

}