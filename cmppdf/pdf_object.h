#pragma once

#include "pdf_array.h"
#include "pdf_dictionary.h"
#include "pdf_stream.h"
#include <iomanip>
#include <vector>
#include <string>
#include <map>

namespace PDF
{
	class Object;

	using string_t = std::string_view;
	using name_t = std::string;
	using array_t = Array;
	using dictionary_t = Dictionary;
	using stream_t = Stream;
	using indirect_t = uint32_t;

	class Object
	{
	public:
		enum class Type
		{
			NIL,
			BOOLEAN,
			NUMERIC,
			STRING,
			NAME,
			ARRAY,
			DICTIONARY,
			STREAM,
			INDIRECT,
		};

	public:
		Object(Type type = Type::NIL);
		Object(bool state);
		Object(double numeric);
		Object(string_t string);
		Object(name_t name);
		Object(array_t array);
		Object(dictionary_t dictionary);
		Object(stream_t stream);
		Object(indirect_t indirect);
		Object(const Object &r);
		Object(Object &&r);
		virtual ~Object();

		Object &operator=(const Object &r);
		Object &operator=(Object &&r);

		operator bool() const noexcept { return m_Type == Type::NIL; }

		bool operator==(const Object &r) const noexcept;
		bool operator!=(const Object &r) const noexcept { return !(*this == r); }
		void diff(std::ostream &out, const Object &r, size_t depth = 0) const;

		bool operator==(Type type) const noexcept { return m_Type == type; }
		bool operator!=(Type type) const noexcept { return m_Type != type; }
		bool operator==(const char *name) const noexcept { return m_Type == Type::NAME && m_Name == name; }
		bool operator!=(const char *name) const noexcept { return m_Type != Type::NAME || m_Name != name; }

		bool HasKey(const char *key) const;
		const Object &operator[](const char *key) const;

		Type GetType() const noexcept { return m_Type; }

		bool GetBoolean() const noexcept { return m_State; }
		double GetNumeric() const noexcept { return m_Numeric; }
		string_t GetString() const noexcept { return m_String; }
		name_t GetName() const noexcept { return m_Name; }
		array_t GetArray() const noexcept { return m_Array; }
		dictionary_t GetDictionary() const noexcept { return m_Dictionary; }
		stream_t GetStream() const noexcept { return m_Stream; }
		indirect_t GetIndirect() const noexcept { return m_Ref; }

		std::string Display() const noexcept;

	private:
		Type m_Type;

		union
		{
			bool m_State;
			double m_Numeric;
			string_t m_String;
			name_t m_Name;
			array_t m_Array;
			dictionary_t m_Dictionary;
			stream_t m_Stream;
			indirect_t m_Ref;
		};

		void Clear();
	};
}

std::ostream &operator<<(std::ostream &out, const PDF::Object &obj);
std::ostream &operator<<(std::ostream &out, const PDF::Object::Type &type);
