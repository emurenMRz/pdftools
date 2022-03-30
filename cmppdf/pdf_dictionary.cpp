#include "pdf_dictionary.h"
#include "pdf_object.h"
#include <sstream>

using namespace PDF;

struct key_store
{
	size_t left = 0, right = 0;
};
using keys_t = std::map<name_t, key_store>;

keys_t compare(const Dictionary &l, const Dictionary &r)
{
	auto keys = keys_t();
	for (const auto &key : l)
		++keys[key.first].left;
	for (const auto &key : r)
		++keys[key.first].right;
	return keys;
}

bool Dictionary::operator==(const Dictionary &r) const noexcept
{
	if (size() != r.size())
		return false;

	for (const auto &key : compare(*this, r))
	{
		if (!key.second.left || !key.second.right)
			return false;
		if (at(key.first) != r.at(key.first))
			return false;
	}
	return true;
}

void Dictionary::diff(std::ostream &out, const Dictionary &r, size_t depth) const noexcept
{
	for (const auto &key : compare(*this, r))
		if (!key.second.left)
			out << std::setw(depth * 4) << ' ' << key.first << ": No key in the left dictionary." << std::endl;
		else if (!key.second.right)
			out << std::setw(depth * 4) << ' ' << key.first << ": No key in the right dictionary." << std::endl;
		else
		{
			auto &lobj = at(key.first);
			auto &robj = r.at(key.first);
			if (lobj != robj)
			{
				out << std::setw(depth * 4) << ' ' << key.first << ":" << std::endl;
				lobj.diff(out, robj, depth + 1);
			}
		}
}

void Dictionary::Merge(const Dictionary &r)
{
	for (const auto &rv : r)
	{
		auto lit = find(rv.first);
		if (lit == end())
			insert(rv);
		else if ((*lit).second != rv.second)
			(*lit).second = std::move(rv.second);
	}
}

bool Dictionary::HasKey(const char *key) const { return find(key) != end(); }
const Object &Dictionary::operator[](const Dictionary::key_type &key) const { return at(key); }
Object &Dictionary::operator[](const Dictionary::key_type &key) { return dictionary_parent_t::operator[](key); }

std::string Dictionary::Display() const noexcept
{
	auto s = std::stringstream();
	s << "<<";
	for (const auto &item : *this)
		s << "/" << item.first << " " << item.second.Display() << " ";
	s << ">>";
	return s.str();
}

/******************************************************************************

******************************************************************************/

std::ostream &operator<<(std::ostream &out, const PDF::Dictionary &dic)
{
	if (dic.HasKey("Type"))
		out << dic["Type"].GetName() << ": " << dic.Display();
	else if (dic.HasKey("Font"))
		out << "Font: " << dic.Display();
	else if (dic.HasKey("CreationDate") || dic.HasKey("ModDate") || dic.HasKey("Producer"))
		out << "Info: " << dic.Display();
	else if (dic.HasKey("Length"))
		out << "Stream?: " << dic.Display();
	else
		out << "*: " << dic.Display();
	return out;
}
