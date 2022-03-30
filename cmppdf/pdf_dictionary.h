#pragma once

#include <ostream>
#include <map>
#include <string>

namespace PDF
{
	class Object;
	using name_t = std::string;
	using dictionary_parent_t = std::map<name_t, Object>;

	class Dictionary : public dictionary_parent_t
	{
	public:
		virtual ~Dictionary() = default;

		bool operator==(const Dictionary &r) const noexcept;
		bool operator!=(const Dictionary &r) const noexcept { return !(*this == r); }
		void diff(std::ostream &out, const Dictionary &r, size_t depth = 0) const noexcept;

		void Merge(const Dictionary &r);

		bool HasKey(const char *key) const;
		const Object &operator[](const Dictionary::key_type &key) const;
		Object &operator[](const Dictionary::key_type &key);

		std::string Display() const noexcept;
	};
}

std::ostream &operator<<(std::ostream &out, const PDF::Dictionary &dic);
