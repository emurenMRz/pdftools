#include "pdf_array.h"
#include "pdf_object.h"
#include <sstream>

using namespace PDF;

bool Array::operator==(const Array &r) const noexcept
{
	if (size() != r.size())
		return false;
	for (auto i = size_t(0); i < size(); ++i)
		if (at(i) != r.at(i))
			return false;
	return true;
}

void Array::diff(std::ostream &out, const Array &r, size_t depth) const noexcept
{
	if (size() != r.size())
		out << "Array size: " << size() << " / " << r.size() << std::endl;
	else
		for (auto i = size_t(0); i < size(); ++i)
		{
			auto &lobj = at(i);
			auto &robj = r.at(i);
			if (lobj != robj)
				lobj.diff(out, robj, depth + 1);
		}
}

std::string Array::Display() const noexcept
{
	auto ss = std::stringstream();
	for (const auto &obj : *this)
		ss << obj.Display() << ' ';
	auto s = ss.str();
	auto length = s.length();
	if (length > 32)
	{
		ss.str("");
		ss.clear(std::ios_base::goodbit);
		ss << ".." << length << "..";
		s = ss.str();
	}
	return "[" + s + "]";
}

/******************************************************************************

******************************************************************************/

std::ostream &operator<<(std::ostream &out, const PDF::Array &array)
{
	out << "Array(" << array.size() << ")";
	return out;
}
