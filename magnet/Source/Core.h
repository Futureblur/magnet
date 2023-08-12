#pragma once

#define MG_VERSION "0.1"

#include "Application.h"

// MG_LOG is a macro that prints the given message to the console.
#define MG_LOG(message) Application::Print("Magnet", message)

// Overload of MG_LOG that prints the given message to the console with the given host.
#define MG_LOG_HOST(host, message) Application::Print(host, message)