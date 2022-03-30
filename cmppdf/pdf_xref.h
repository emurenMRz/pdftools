#pragma once

#include "pdf_object.h"

namespace PDF
{
	struct Xref
	{
		long offset = 0;
		int revision = 0;
		bool used = 0;

		Object object;
		stream_t stream;

		bool operator==(const Xref &r) const { return /*offset == r.offset &&*/ revision == r.revision && used == r.used && object == r.object && stream == r.stream; }
		bool operator!=(const Xref &r) const { return !(*this == r); }

		void diff(std::ostream &out, const Xref &r, size_t depth = 0) const;
	};
}

std::ostream &operator<<(std::ostream &out, const PDF::Xref &xref);
