#pragma once

#include <stdexcept>

namespace PDF
{
	class parse_error : public std::logic_error
	{
	public:
		parse_error(const std::string &message) : std::logic_error(message.c_str()) {}
		parse_error(const char *message) : std::logic_error(message) {}
	};

	class type_error : public std::runtime_error
	{
	public:
		type_error(const std::string &message) : std::runtime_error(message.c_str()) {}
		type_error(const char *message) : std::runtime_error(message) {}
	};

	class reference_error : public std::runtime_error
	{
	public:
		reference_error(const std::string &message) : std::runtime_error(message.c_str()) {}
		reference_error(const char *message) : std::runtime_error(message) {}
	};
}