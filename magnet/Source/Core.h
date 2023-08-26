#pragma once

#define MG_VERSION "0.4.2"

#include "Application.h"

// Prints the given message to the console using the default Magnet host.
#define MG_LOG(message) Application::Print("Magnet", message)

// Prints the given message to the console without a host.
#define MG_LOGNH(message) Application::Print("", message)

// Overload of MG_LOG that prints the given message to the console with the given host.
#define MG_LOG_HOST(host, message) Application::Print(host, message)