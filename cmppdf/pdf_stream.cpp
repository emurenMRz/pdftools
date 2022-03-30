#pragma once

#include "pdf_stream.h"
#include <sstream>

using namespace PDF;

Stream &Stream::operator=(const Stream &r)
{
	m_Begin = r.m_Begin;
	m_Size = r.m_Size;
	return *this;
}

Stream &Stream::operator=(Stream &&r)
{
	m_Begin = r.m_Begin;
	m_Size = r.m_Size;
	return *this;
}

bool Stream::operator==(const Stream &r) const noexcept
{
	if (m_Size != r.m_Size)
		return false;
	const auto s = reinterpret_cast<uint8_t *>(m_Begin);
	const auto d = reinterpret_cast<uint8_t *>(r.m_Begin);
	return !std::memcmp(s, d, m_Size);
}

void Stream::diff(std::ostream &out, const Stream &r, size_t depth) const noexcept
{
	if (m_Size != r.m_Size)
		out << std::setw(depth * 4) << ' ' << "Size: " << m_Size << " / " << r.m_Size << std::endl;
	else
	{
		const auto s = reinterpret_cast<uint8_t *>(m_Begin);
		const auto d = reinterpret_cast<uint8_t *>(r.m_Begin);
		for (auto i = 0; i < m_Size; ++i)
			if (s[i] != d[i])
			{
				out << std::setw(depth * 4) << ' ' << "Offset[" << i << "]" << std::endl;
				break;
			}
	}
}

std::string Stream::Display() const noexcept
{
	auto s = std::stringstream();
	s << std::hex << m_Begin << '-' << std::hex << (m_Begin + m_Size);
	return s.str();
}

/******************************************************************************

******************************************************************************/

std::ostream &operator<<(std::ostream &out, const Stream &view)
{
	out << view.GetSize();
	return out;
}