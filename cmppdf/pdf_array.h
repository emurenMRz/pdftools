#pragma once

#include <ostream>
#include <vector>

namespace PDF
{
	class Object;
	using array_parent_t = std::vector<Object>;

	class Array : public array_parent_t
	{
	public:
		virtual ~Array() = default;

		bool operator==(const Array &r) const noexcept;
		bool operator!=(const Array &r) const noexcept { return !(*this == r); }
		void diff(std::ostream &out, const Array &r, size_t depth = 0) const noexcept;

		std::string Display() const noexcept;
	};
}

std::ostream &operator<<(std::ostream &out, const PDF::Array &array);
