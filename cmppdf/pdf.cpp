#include "pdf.h"
#include <sstream>
#include <regex>

using namespace PDF;

#define WHITESPACE "\f\t\r\n "
#define DELIMITER "()<>[]{}/%"
#define WSD (WHITESPACE DELIMITER)

inline bool is_hex(char ch) { return ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F' || ch >= 'a' && ch <= 'f'; }
inline bool is_whitespace(char ch) { return !ch || ch == '\f' || ch == '\t' || ch == '\r' || ch == '\n' || ch == ' '; }

Document::Document(std::string_view name)
	: FileImage(name)
{
	if (!Analyze())
		throw parse_error("Not PDF");
}

bool Document::operator==(const Document &r) const
{
	if (m_Version != r.m_Version)
		return false;

	if (m_XrefTable.size() != r.m_XrefTable.size())
		return false;

	auto table_size = m_XrefTable.size();
	for (auto i = decltype(table_size)(0); i < table_size; ++i)
		if (m_XrefTable[i] != r.m_XrefTable[i])
			return false;

	// File Trailer
	if (m_FileTrailer.size != r.m_FileTrailer.size)
		return false;
	if (m_FileTrailer.root != r.m_FileTrailer.root)
		return false;
	if (m_FileTrailer.info != r.m_FileTrailer.info)
		return false;

	return true;
}

void Document::diff(std::ostream &out, const Document &r) const
{
	if (m_Version != r.m_Version)
		out << "Version: " << m_Version << " / " << r.m_Version << std::endl;

	if (m_XrefTable.size() != r.m_XrefTable.size())
		out << "Xref table size: " << m_XrefTable.size() << " / " << r.m_XrefTable.size() << std::endl;
	else
	{
		auto table_size = m_XrefTable.size();
		for (auto i = decltype(table_size)(0); i < table_size; ++i)
		{
			auto &lo = m_XrefTable[i];
			auto &ro = r.m_XrefTable[i];
			if (lo != ro)
			{
				out << "Xref table [" << i << "]" << std::endl;
				lo.diff(out, ro, 1);
			}
		}
	}

	// File Trailer
	if (m_FileTrailer.size != r.m_FileTrailer.size)
		out << "File trailer size: " << m_FileTrailer.size << " / " << r.m_FileTrailer.size << std::endl;
}

bool Document::Analyze()
{
	// The first line is PDF version.
	// e.g) %PDF-1.7
	auto line = GetLine();
	if (line.empty() || line.substr(0, 5) != "%PDF-")
		return false;

	m_Version = line.substr(5);

	// It is recommended that the second line should be embedded as a
	// comment (%) with appropriate code to be recognised as a binary
	// file, but in case it is not embedded, treat it as a normal comment.

	// The PDF ends with %%EOF.
	Seek(FileImage::End);
	line = GetLineBack();
	if (line.empty() || line != "%%EOF")
		return false;

	// Cross-reference table address
	line = GetLineBack();
	if (line.empty())
		return false;

	auto xref_offset = strtoul(line.data(), NULL, 10);

	// Begin tag of Cross-reference table
	line = GetLineBack();
	if (line.empty() || line != "startxref")
		return false;

	// Parsing Cross-reference table
	Seek(xref_offset);
	m_XrefTable.clear();

	ParseXrefTable();

	// Pre-decode objects.
	for (auto &xref : m_XrefTable)
		GetObject(xref);

	// Recursively traversing objects.
	// ParseCatalog(m_FileTrailer.root);

	return true;
}

/******************************************************************************

******************************************************************************/

/**
 * TABLE 3.25 Entries in the catalog dictionary
 */
void Document::ParseCatalog(dictionary_t &dic)
{
	if (dic["Type"] != "Catalog")
		throw parse_error("not Catalog");

	std::cout << "Catalog: " << dic.Display() << std::endl;

	// Optional; PDF 1.4
	// dic["Version"] // Name

	if (dic.HasKey("Outlines"))
		ParseOutlines(GetIndirectObject(dic["Outlines"]));
	if (dic.HasKey("Metadata"))
		ParseMetadata(GetIndirectObject(dic["Metadata"]));
	ParsePages(GetIndirectObject(dic["Pages"]));
}

/**
 *
 */
void Document::ParseOutlines(dictionary_t &dic)
{
	if (dic["Type"] != "Outlines")
		throw parse_error("not Outlines");

	std::cout << "Outlines: " << dic.Display() << std::endl;
}

void Document::ParseMetadata(dictionary_t &dic)
{
	if (dic["Type"] != "Metadata")
		throw parse_error("not Metadata");

	std::cout << "Metadata: " << dic.Display() << std::endl;
}

/**
 * TABLE 3.26 Required entries in a page tree node
 */
void Document::ParsePages(dictionary_t &dic)
{
	if (dic["Type"] != "Pages")
		throw parse_error("not Pages");

	std::cout << "Pages: " << dic.Display() << std::endl;

	auto kids = dic["Kids"].GetArray();
	auto count = dic["Count"].GetNumeric();
	if (kids.size() != size_t(count))
		throw parse_error("failed count of pages");

	for (auto &&page : kids)
		ParsePage(GetObject(page.GetIndirect()).object.GetDictionary());
}

/**
 * TABLE 3.27 Entries in a page object
 */
void Document::ParsePage(dictionary_t &dic)
{
	if (dic["Type"] != "Page")
		throw parse_error("not Page");

	std::cout << "Page: " << dic.Display() << std::endl;

	auto resources = dic["Resources"];
	if (resources == Object::Type::DICTIONARY)
		;
	else if (resources == Object::Type::INDIRECT)
		resources = GetObject(resources.GetIndirect()).object;
	ParseResources(resources.GetDictionary());

	if (dic.HasKey("Contents"))
	{
		auto contents = dic["Contents"];
		if (contents == Object::Type::INDIRECT)
			contents = GetObject(contents.GetIndirect()).object;
		else if (contents == Object::Type::ARRAY)
			throw parse_error("unimplement");
	}
}

void Document::ParseResources(dictionary_t &dic)
{
	std::cout << "Resources: " << dic.Display() << std::endl;

	if (dic.HasKey("Font"))
	{
		auto font = dic["Font"].GetDictionary();
		for (const auto &item : font)
		{
			if (item.second == Object::Type::DICTIONARY)
				ParseFont(item.second.GetDictionary());
			else if (item.second == Object::Type::INDIRECT)
				ParseFont(GetObject(item.second.GetIndirect()).object.GetDictionary());
		}
	}
	if (dic.HasKey("ProcSet"))
	{
		auto proc_set = dic["ProcSet"].GetArray();
	}
}

void Document::ParseFont(dictionary_t &dic)
{
	if (dic["Type"] != "Font")
		throw parse_error("not Font");

	std::cout << "Font: " << dic.Display() << std::endl;
}

/******************************************************************************

******************************************************************************/

void Document::ParseXrefTable()
{
	// Check begin tag
	auto line = std::string(GetLine());
	if (line.empty() || line != "xref")
		throw std::logic_error("need xref token.");

	// Subsections
	for (;;)
	{
		line = GetLine();
		if (line.empty())
			throw std::logic_error("need cross-reference entry or 'trailer' keyword.");

		if (line == "trailer")
			break;

		// Begin number and number of sessions in table: begin_no sessions
		static auto offset_reg = std::regex(R"(([0-9]+) ([0-9]+))");
		auto m = std::smatch();

		if (!std::regex_match(line, m, offset_reg))
			continue;

		auto begin = strtoul(m[1].str().c_str(), nullptr, 10);
		auto count = strtoul(m[2].str().c_str(), nullptr, 10);

		if (begin + count >= m_XrefTable.size())
			m_XrefTable.resize(begin + count);

		static auto xref_reg = std::regex(R"(^([0-9]{10}) ([0-9]{5}) ([fn])[ \r][\r\n])");
		for (auto i = decltype(count)(0); i < count; ++i)
		{
			auto m = GetLine(xref_reg);
			if (m.size() != 4)
				throw std::logic_error("need offset.");

			auto &xref = m_XrefTable[begin + i];
			xref.offset = strtoul(m[1].str().c_str(), nullptr, 10);
			xref.revision = strtoul(m[2].str().c_str(), nullptr, 10);
			xref.used = m[3] == 'n';
		}
	}

	// File Trailer
	auto trailer = Parse();
	if (trailer != Object::Type::DICTIONARY)
		throw parse_error("Need dictionary");

	// Required; must not be an indirect reference
	auto size = size_t(trailer["Size"].GetNumeric());
	if (size > m_FileTrailer.size)
		m_FileTrailer.size = size;

	// Present only if the file has more than one cross-reference section; must not be an indirect reference
	if (trailer.HasKey("Prev"))
	{
		Seek(size_t(trailer["Prev"].GetNumeric()));
		ParseXrefTable();
	}

	// Required if document is encrypted; PDF 1.1
	// if(trailer.HasKey("Encrypt")){}

	// Optional; must be an indirect reference
	if (trailer.HasKey("Info"))
	{
		if (!m_FileTrailer.info.empty())
			throw parse_error("dupplicated Info item");
		m_FileTrailer.info = GetIndirectObject(trailer["Info"]);
	}

	// Optional, but strongly recommended; PDF 1.1
	// dic.find("ID");

	// Required; must be an indirect reference
	if (trailer.HasKey("Root"))
	{
		if (!m_FileTrailer.root.empty())
			throw parse_error("dupplicated Root item");
		m_FileTrailer.root = GetIndirectObject(trailer["Root"]);
	}
}

/******************************************************************************

******************************************************************************/

Xref &Document::GetObject(size_t obj_no)
{
	if (obj_no >= m_XrefTable.size())
		throw std::out_of_range("Need cross-reference table size");
	return GetObject(m_XrefTable[obj_no]);
}

Xref &Document::GetObject(Xref &xref)
{
	static auto dummy = Xref();

	if (!xref.used)
		return dummy;
	if (xref.object != Object::Type::NIL)
		return xref;

	Seek(xref.offset);

	// Check begin tag: object_no revision_no 'obj'
	auto m = GetLine(std::regex(R"(^([0-9]+) ([0-9]+) obj)"));
	if (m.size() != 3)
		throw std::logic_error("unknown object header format");

	auto no = strtoul(m[1].str().c_str(), nullptr, 10);
	auto rev = strtoul(m[2].str().c_str(), nullptr, 10);

	// body
	xref.object = Parse();

	auto line = GetLine();
	if (line == "stream")
	{
		auto length = xref.object["Length"];
		auto fp = Tell();
		auto begin = uintptr_t(Image().data() + fp);
		auto size = size_t(0);
		if (length == Object::Type::NUMERIC)
			size = size_t(length.GetNumeric());
		else if (length == Object::Type::INDIRECT)
		{
			auto &r = GetObject(length.GetIndirect());
			if (r.object != Object::Type::NUMERIC)
				throw parse_error("Need numeric type");
			size = size_t(r.object.GetNumeric());
			Seek(fp);
		}
		else
			throw parse_error("Need Length");
		xref.stream = stream_t{begin, size};
		Skip(size);

		line = GetLine();
		if (line != "endstream")
			throw std::logic_error("endress stream...");
		line = GetLine();
	}

	// Check end tag
	if (line != "endobj")
		throw std::logic_error("endress object...");

	return xref;
}

/******************************************************************************

******************************************************************************/

Document::Token Document::Lex()
{
	Skip();

	// skip comment.
	if (GetCH() == '%')
	{
		Skip("\r\n");
		Skip();
	}

	switch (GetCH())
	{
	case 't':
		if (GetLine(WSD) == "true")
			return Token(Object(true));
		throw parse_error("Unknown Token");

	case 'f':
		if (GetLine(WSD) == "false")
			return Token(Object(false));
		throw parse_error("Unknown Token");

	case '+':
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{
		static auto r = std::regex(R"(^([0-9]+) ([0-9]+) R)");
		auto m = GetLine(r);
		if (m.ready() && !m.empty())
		{
			auto t1 = strtoul(m[1].str().c_str(), nullptr, 10);
			auto t2 = strtoul(m[2].str().c_str(), nullptr, 10);
			if (t2 > 0)
				throw parse_error("No support for non-zero generation.");
			return Token(Object(indirect_t{t1}));
		}
		return Token(Object(strtod(GetLine(WSD).data(), nullptr)));
	}

	case '(':
	{
		auto begin = Tell();
		auto stack = 0;
		for (;;)
		{
			auto ch = Get();
			if (ch == '\\')
			{
				ch = Get();
				if (std::string_view("nrtbf()\\").find_first_of(ch) != std::string_view::npos)
					; // escape
				else if (ch >= '0' && ch <= '9')
				{
					auto m = Check(std::regex(R"(([0-9][1,4]))"));
					// Oct 3-digit(\ddd) or 2-digit(\dd)
					if ((ch = Get()) < '0' || ch > '9')
						throw std::logic_error("");
					if ((ch = Get()) < '0' || ch > '9')
						Unget();
				}
				else
					Unget();
			}
			else if (ch == '(')
				++stack;
			else if (ch == ')')
				if (--stack == 0)
					break;
		}
		return Token(Object(string_t{Image().data() + begin, Tell() - begin}));
	}

	case '<':
		if (Check("<<"))
			return Token(TokenType::DictionaryBegin);
		else
		{
			auto begin = Tell();
			auto stack = 0;
			for (;;)
			{
				auto ch = Get();
				if (is_hex(ch))
					;
				else if (ch == '<')
					++stack;
				else if (ch == '>')
				{
					if (--stack == 0)
						break;
				}
				else if (!is_whitespace(ch))
					throw parse_error("Failed parse hex string.");
			}
			return Token(Object(string_t{Image().data() + begin, Tell() - begin}));
		}

	case '>':
		if (Check(">>"))
			return Token(TokenType::DictionaryEnd);
		throw parse_error("Unknown Token");

	case '/':
		Get();
		return Token(Object(name_t{GetLine(WSD)}));

	case '[':
		Get();
		return Token(TokenType::ArrayBegin);
	case ']':
		Get();
		return Token(TokenType::ArrayEnd);

	case 's':
		if (Check("stream"))
		{
			// Only CRLF or LF line breaks are permitted on the start-of-stream line, so skip to LF.
			while (Get() != '\n')
				;
			return Token(TokenType::StreamBegin);
		}
		throw parse_error("Unknown Token");

	case 'e':
		if (Check("endstream"))
			return Token(TokenType::StreamEnd);
		if (Check("endobj"))
			return Token(TokenType::ObjectEnd);
		throw parse_error("Unknown Token");
	}

	throw parse_error("Unknown Token");
}

Object Document::Parse(Token stock)
{
	auto token = !stock.type ? Lex() : stock;
	stock.type = 0;

	switch (token.type)
	{
	case TokenType::ArrayBegin:
	{
		auto array = array_t();
		for (;;)
		{
			auto value = Lex();
			if (value == TokenType::ArrayEnd)
				break;
			if (value.type == TokenType::ArrayBegin || value.type == TokenType::DictionaryBegin)
				array.emplace_back(Parse(value));
			else
				array.emplace_back(value.object);
		}
		return Object(array);
	}

	case TokenType::DictionaryBegin:
	{
		auto dic = dictionary_t();
		for (;;)
		{
			auto name = Lex();
			if (name == TokenType::DictionaryEnd)
				break;
			if (name != int(Object::Type::NAME))
				throw parse_error("need Name.");
			auto value = Lex();
			if (value.type == TokenType::ArrayBegin || value.type == TokenType::DictionaryBegin)
				dic[name.object.GetName()] = Parse(value);
			else
				dic[name.object.GetName()] = value.object;
		}
		return Object(dic);
	}

	default:
		return token.object;
	}
}

/******************************************************************************

******************************************************************************/

std::ostream &operator<<(std::ostream &out, const PDF::Document &doc)
{
	out << "pdf version: " << doc.GetVersion() << std::endl
		<< std::setw(10) << "no" << ' ' << std::setw(10) << "xref" << ' ' << std::setw(5) << "rev" << ' ' << std::setw(6) << "used"
		<< " object" << std::endl;

	auto index = 0;
	for (const auto &xref : doc.GetXrefTable())
	{
		out << std::setw(10) << index << ' ' << xref << std::endl;
		++index;
	}
	return out;
}
