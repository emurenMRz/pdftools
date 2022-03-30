#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <regex>

class FileImage
{
public:
	static const size_t End = -1;
	using image_t = std::vector<char>;

public:
	FileImage(std::string_view name);

	void Seek(size_t pos);
	size_t Tell() const noexcept { return m_FP; }

	void Skip();
	void Skip(size_t step);
	void Skip(std::string_view delim);

	char GetCH() const { return m_Image[m_FP]; }
	char Get() { return m_Image[m_FP++]; }
	void Unget() { --m_FP; }

	bool Check(std::string_view token, bool step = true);
	std::cmatch Check(std::regex reg);

	std::string_view GetLine(size_t size, bool step = true);
	std::string_view GetLine(std::string_view delim, bool step = true);
	std::cmatch GetLine(std::regex reg, bool step = true);
	
	std::string_view GetLine();
	std::string_view GetLineBack();

	const image_t &Image() const { return m_Image; }

private:
	image_t m_Image;
	size_t m_FP;
};
