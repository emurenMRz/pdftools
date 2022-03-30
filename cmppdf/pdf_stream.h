#pragma once

#include <iomanip>
#include <cstdint>
#include <cstring>

namespace PDF
{
	class Stream
	{
	public:
		Stream() : m_Begin(0), m_Size(0) {}
		Stream(uintptr_t begin, size_t size) : m_Begin(begin), m_Size(size) {}
		Stream(const Stream &r) : m_Begin(r.m_Begin), m_Size(r.m_Size) {}
		Stream(Stream &&r) : m_Begin(r.m_Begin), m_Size(r.m_Size) {}

		Stream &operator=(const Stream &r);
		Stream &operator=(Stream &&r);

		bool operator==(const Stream &r) const noexcept;
		bool operator!=(const Stream &r) const noexcept { return !(*this == r); }
		void diff(std::ostream &out, const Stream &r, size_t depth = 0) const noexcept;

		size_t GetSize() const noexcept { return m_Size; }

		std::string Display() const noexcept;

	private:
		uintptr_t m_Begin;
		size_t m_Size;
	};
}

std::ostream &operator<<(std::ostream &out, const PDF::Stream &stream);