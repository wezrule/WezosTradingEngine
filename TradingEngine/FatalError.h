#pragma once

#include "Error.h"

#include <stdexcept>

class FatalError : public std::runtime_error {
public:
	FatalError(Error::Type err) :
	FatalError(err, "") {
	}

	FatalError(Error::Type err, const char* what) :
	std::runtime_error(what),
	err(err) {
	}

	Error::Type GetType() const {
		return err;
	}

private:
	Error::Type err;
};
