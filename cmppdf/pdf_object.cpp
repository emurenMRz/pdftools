#include "pdf_object.h"
#include "pdf_except.h"
#include <sstream>

using namespace PDF;

Object::Object(Object::Type type) : m_Type(type) {}
Object::Object(bool state) : m_Type(Type::BOOLEAN), m_State(state) {}
Object::Object(double numeric) : m_Type(Type::NUMERIC), m_Numeric(numeric) {}
Object::Object(string_t string) : m_Type(Type::STRING), m_String(string) {}
Object::Object(name_t name) : m_Type(Type::NAME), m_Name(name) {}
Object::Object(array_t array) : m_Type(Type::ARRAY), m_Array(array) {}
Object::Object(dictionary_t dictionary) : m_Type(Type::DICTIONARY), m_Dictionary(dictionary) {}
Object::Object(stream_t stream) : m_Type(Type::STREAM), m_Stream(stream) {}
Object::Object(indirect_t ref) : m_Type(Type::INDIRECT), m_Ref(ref) {}

Object::Object(const Object &r) { *this = r; }
Object::Object(Object &&r) { *this = std::move(r); }

Object::~Object() { Clear(); }

Object &Object::operator=(const Object &r)
{
#define assign(var) new (&var) decltype(var)(r.var);
	Clear();
	m_Type = r.m_Type;
	switch (m_Type)
	{
	case Type::BOOLEAN:
		m_State = r.m_State;
		break;
	case Type::NUMERIC:
		m_Numeric = r.m_Numeric;
		break;
	case Type::STRING:
		assign(m_String);
		break;
	case Type::NAME:
		assign(m_Name);
		break;
	case Type::ARRAY:
		assign(m_Array);
		break;
	case Type::DICTIONARY:
		assign(m_Dictionary);
		break;
	case Type::STREAM:
		m_Stream = r.m_Stream;
		break;
	case Type::INDIRECT:
		m_Ref = r.m_Ref;
		break;
	}
#undef assign
	return *this;
}

Object &Object::operator=(Object &&r)
{
#define assign(var)                 \
	{                               \
		new (&var) decltype(var)(); \
		var.swap(r.var);            \
	}
	m_Type = r.m_Type;
	switch (m_Type)
	{
	case Type::BOOLEAN:
		m_State = r.m_State;
		break;
	case Type::NUMERIC:
		m_Numeric = r.m_Numeric;
		break;
	case Type::STRING:
		assign(m_String);
		break;
	case Type::NAME:
		assign(m_Name);
		break;
	case Type::ARRAY:
		assign(m_Array);
		break;
	case Type::DICTIONARY:
		assign(m_Dictionary);
		break;
	case Type::STREAM:
		m_Stream = r.m_Stream;
		break;
	case Type::INDIRECT:
		m_Ref = r.m_Ref;
		break;
	}
	r.Clear();
#undef assign
	return *this;
}

bool Object::operator==(const Object &r) const noexcept
{
	if (m_Type != r.m_Type)
		return false;
	switch (m_Type)
	{
	case Type::NIL:
		return true;
	case Type::BOOLEAN:
		return m_State == r.m_State;
	case Type::NUMERIC:
		return m_Numeric == r.m_Numeric;
	case Type::STRING:
		return m_String == r.m_String;
	case Type::NAME:
		return m_Name == r.m_Name;
	case Type::ARRAY:
		return m_Array == r.m_Array;
	case Type::DICTIONARY:
		return m_Dictionary == r.m_Dictionary;
	case Type::STREAM:
		return m_Stream == r.m_Stream;
	case Type::INDIRECT:
		return m_Ref == r.m_Ref;
	default:
		return false;
	}
}

void Object::diff(std::ostream &out, const Object &r, size_t depth) const
{
#define check(item, name)                                                                           \
	if (name != r.name)                                                                             \
	{                                                                                               \
		out << std::setw(depth * 4) << ' ' << item << ": " << name << " / " << r.name << std::endl; \
	}

	check("Type", m_Type) else switch (m_Type)
	{
	case Type::BOOLEAN:
		check("Boolean", m_State) break;
	case Type::NUMERIC:
		check("Numeric", m_Numeric) break;
	case Type::STRING:
		check("String", m_String) break;
	case Type::NAME:
		check("Name", m_Name) break;
	case Type::ARRAY:
		m_Array.diff(out, r.m_Array, depth);
		break;
	case Type::DICTIONARY:
		m_Dictionary.diff(out, r.m_Dictionary, depth);
		break;
	case Type::STREAM:
		m_Stream.diff(out, r.m_Stream, depth);
		break;
	case Type::INDIRECT:
		check("Indirect", m_Ref) break;
	}
#undef check
}

bool Object::HasKey(const char *key) const
{
	if (m_Type != Type::DICTIONARY)
		throw type_error(__FILE__);
	return m_Dictionary.HasKey(key);
}

const Object &Object::operator[](const char *key) const
{
	if (m_Type != Type::DICTIONARY)
		throw type_error(__FILE__);
	return m_Dictionary[key];
}

std::string Object::Display() const noexcept
{
	auto s = std::stringstream();
	switch (GetType())
	{
	case PDF::Object::Type::NIL:
		s << "null";
		break;
	case PDF::Object::Type::BOOLEAN:
		s << GetBoolean();
		break;
	case PDF::Object::Type::NUMERIC:
		s << GetNumeric();
		break;
	case PDF::Object::Type::STRING:
		s << std::string(GetString());
		break;
	case PDF::Object::Type::NAME:
		s << '/' << GetName();
		break;
	case PDF::Object::Type::ARRAY:
		s << GetArray().Display();
		break;
	case PDF::Object::Type::DICTIONARY:
		s << GetDictionary().Display();
		break;
	case PDF::Object::Type::STREAM:
		s << GetStream().Display();
		break;
	case PDF::Object::Type::INDIRECT:
		s << GetIndirect() << " 0 R";
		break;
	}

	return s.str();
}

void Object::Clear()
{
	switch (m_Type)
	{
	case Type::STRING:
		m_String.~string_t();
		break;
	case Type::NAME:
		m_Name.~name_t();
		break;
	case Type::ARRAY:
		m_Array.~array_t();
		break;
	case Type::DICTIONARY:
		m_Dictionary.~dictionary_t();
		break;
	}
	m_Type = Type::NIL;
}

/******************************************************************************

******************************************************************************/

std::ostream &operator<<(std::ostream &out, const PDF::Object &obj)
{
	switch (obj.GetType())
	{
	case PDF::Object::Type::NIL:
		out << "null";
		break;
	case PDF::Object::Type::BOOLEAN:
		out << obj.GetBoolean();
		break;
	case PDF::Object::Type::NUMERIC:
		out << obj.GetNumeric();
		break;
	case PDF::Object::Type::STRING:
		out << std::string(obj.GetString());
		break;
	case PDF::Object::Type::NAME:
		out << obj.GetName();
		break;
	case PDF::Object::Type::ARRAY:
		out << obj.GetArray();
		break;
	case PDF::Object::Type::DICTIONARY:
		out << obj.GetDictionary();
		break;
	case PDF::Object::Type::STREAM:
		out << obj.GetStream();
		break;
	case PDF::Object::Type::INDIRECT:
		out << obj.GetIndirect();
		break;
	}
	return out;
}

std::ostream &operator<<(std::ostream &out, const PDF::Object::Type &type)
{
#define cast_type(type, msg)      \
	case PDF::Object::Type::type: \
		out << msg;               \
		break;

	switch (type)
	{
		cast_type(NIL, "NIL");
		cast_type(BOOLEAN, "BOOLEAN");
		cast_type(NUMERIC, "NUMERIC");
		cast_type(STRING, "STRING");
		cast_type(NAME, "NAME");
		cast_type(ARRAY, "ARRAY");
		cast_type(DICTIONARY, "DICTIONARY");
		cast_type(STREAM, "STREAM");
		cast_type(INDIRECT, "INDIRECT");
	}
	return out;
#undef case_type
}
