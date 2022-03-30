#pragma once

#include "file_image.h"
#include "pdf_except.h"
#include "pdf_xref.h"
#include "pdf_object.h"
#include <iomanip>
#include <string>
#include <vector>

namespace PDF
{
	class Document : public FileImage
	{
	public:
		Document(std::string_view name);

		bool operator==(const Document &r) const;
		void diff(std::ostream &out, const Document &r) const;

		bool Analyze();

		std::string_view GetVersion() const { return m_Version; }
		std::vector<Xref> GetXrefTable() const { return m_XrefTable; }

	private:
		enum TokenType
		{
			ArrayBegin = 0x100,
			ArrayEnd,
			DictionaryBegin,
			DictionaryEnd,
			StreamBegin,
			StreamEnd,
			ObjectEnd,
		};

		struct Token
		{
			int type;
			Object object;

			Token(int type = int(Object::Type::NIL)) : type(type) {}
			Token(Object object) : object(object) { type = int(object.GetType()); }

			bool operator==(int r) const noexcept { return type == r; }
			bool operator!=(int r) const noexcept { return type != r; }
		};

		std::string_view m_Version;
		std::vector<Xref> m_XrefTable;

		struct
		{
			size_t size = 0;
			dictionary_t root;
			dictionary_t info;
		} m_FileTrailer;

		void ParseXrefTable();
		void ParseCatalog(dictionary_t &dic);
		void ParseOutlines(dictionary_t &dic);
		void ParseMetadata(dictionary_t &dic);
		void ParsePages(dictionary_t &dic);
		void ParsePage(dictionary_t &dic);
		void ParseResources(dictionary_t &dic);
		void ParseFont(dictionary_t &dic);

		Xref &GetObject(size_t obj_no);
		Xref &GetObject(Xref &xref);
		dictionary_t GetIndirectObject(const Object &obj) { return GetObject(obj.GetIndirect()).object.GetDictionary(); }

		Token Lex();
		Object Parse(Token stock = Token());
	};
}

std::ostream &operator<<(std::ostream &out, const PDF::Document &doc);
