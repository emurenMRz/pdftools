#include "pdf_xref.h"

using namespace PDF;

void Xref::diff(std::ostream &out, const Xref &r, size_t depth) const
{
	// if (offset != r.offset)
	// 	out << std::setw(depth * 4) << ' ' << "Offset: " << offset << " / " << r.offset << std::endl;
	if (revision != r.revision)
		out << std::setw(depth * 4) << ' ' << "Revision: " << revision << " / " << r.revision << std::endl;
	if (used != r.used)
		out << std::setw(depth * 4) << ' ' << "Used: " << used << " / " << r.used << std::endl;

	if (object != r.object)
	{
		out << std::setw(depth * 4) << ' ' << "Object: " << std::endl;
		object.diff(out, r.object, depth + 1);
	}
	if (stream != r.stream)
	{
		out << std::setw(depth * 4) << ' ' << "Stream: " << std::endl;
		stream.diff(out, r.stream, depth + 1);
	}
}

/******************************************************************************

******************************************************************************/

std::ostream &operator<<(std::ostream &out, const PDF::Xref &xref)
{
	out << std::setw(10) << xref.offset << ' ' << std::setw(5) << xref.revision << ' ' << std::setw(6) << (xref.used ? "use" : "unused") << ' ' << xref.object;
	if (xref.stream.GetSize() > 0)
		out << " stream[" << xref.stream.GetSize() << "]";
	return out;
}
