#include "file_image.h"
#include <fstream>

FileImage::FileImage(std::string_view name)
{
	auto in = std::ifstream(name.data(), std::ios_base::binary);
	m_Image.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	m_FP = 0;
}

void FileImage::Seek(size_t pos)
{
	if (pos >= m_Image.size())
		pos = m_Image.size() - 1;
	m_FP = pos;
}

void FileImage::Skip()
{
	const auto zero = m_Image.data();
	const auto max = zero + m_Image.size();
	auto p = zero + m_FP;
	for (; p < max && (!*p || *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == '\f'); ++p)
		;
	m_FP = p - zero;
}

void FileImage::Skip(size_t step)
{
	const auto zero = m_Image.data();
	const auto max = zero + m_Image.size();
	auto p = zero + m_FP;
	p += step;
	if (p >= max)
		throw std::out_of_range("not found head.");
	m_FP = p - zero;
}

void FileImage::Skip(std::string_view delim)
{
	const auto zero = m_Image.data();
	const auto max = zero + m_Image.size();
	auto head = zero + m_FP;
	auto tail = head;
	for (; delim.find_first_of(*tail) == delim.npos; ++tail)
		if (tail >= max)
			break;
	m_FP = tail - zero;
}

bool FileImage::Check(std::string_view token, bool step)
{
	if (std::string_view(&m_Image[m_FP], token.size()) != token)
		return false;
	if (step)
		m_FP += token.size();
	return true;
}

std::cmatch FileImage::Check(std::regex reg)
{
	auto m = std::cmatch();
	std::regex_search(&m_Image[m_FP], m, reg);
	return m;
}

std::string_view FileImage::GetLine(size_t size, bool step)
{
	Skip();
	const auto zero = m_Image.data();
	const auto max = zero + m_Image.size();
	auto head = zero + m_FP;
	if (head + size > max)
		size = max - head;
	if (step)
		m_FP += size;
	return std::string_view(head, size);
}

std::string_view FileImage::GetLine(std::string_view delim, bool step)
{
	Skip();
	const auto zero = m_Image.data();
	const auto max = zero + m_Image.size();
	auto head = zero + m_FP;
	auto tail = head;
	for (; tail < max && delim.find_first_of(*tail) == delim.npos; ++tail)
		;
	if (step)
		m_FP = tail - zero;
	return std::string_view(head, tail - head);
}

std::cmatch FileImage::GetLine(std::regex reg, bool step)
{
	Skip();
	auto m = std::cmatch();
	if (std::regex_search(&m_Image[m_FP], m, reg))
		if (step)
			m_FP += m.position(0) + m.length(0);
	return m;
}

std::string_view FileImage::GetLine()
{
	Skip();
	const auto zero = m_Image.data();
	const auto max = zero + m_Image.size();
	auto head = zero + m_FP;
	auto tail = head;
	for (; tail < max && *tail != '\r' && *tail != '\n'; ++tail)
		;
	for (m_FP = tail - zero; m_Image[m_FP] == '\r' || m_Image[m_FP] == '\n'; ++m_FP)
		;
	return std::string_view(head, tail - head);
}

std::string_view FileImage::GetLineBack()
{
	const auto zero = m_Image.data();
	auto tail = zero + m_FP;
	for (; *tail == '\r' || *tail == '\n'; --tail)
		if (tail < zero)
			throw std::out_of_range("not found tail.");
	auto head = tail;
	for (; *head != '\r' && *head != '\n'; --head)
		if (head < zero)
			throw std::out_of_range("not found head.");
	m_FP = head - zero;
	return std::string_view(head + 1, tail - head);
}
