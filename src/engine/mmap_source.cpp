#include "mmap_source.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace swf_redux
{

namespace io
{

class MMapSource_impl
{
public:
	MMapSource_impl(const wchar_t * a_FilePath);
	~MMapSource_impl();

	uint64_t GetSize() const		{ return m_FileSize.QuadPart; }
	uint64_t FileOffset() const		{ return m_Offset; }
	bool	 Eof() const			{ return !(m_Offset < GetSize()); }
	bool	 Read(void * a_Dst, size_t a_Count);
	bool	 Skip(size_t a_Count);

protected:
	HANDLE m_FileHandle;
	HANDLE m_FileMapping;
	const void * m_Data;
	LARGE_INTEGER m_FileSize;
	uint64_t m_Offset;
};

MMapSource_impl::MMapSource_impl(const wchar_t * a_FilePath) : m_Offset(0)
{
	m_FileHandle = CreateFile(a_FilePath,
		GENERIC_READ,
		FILE_SHARE_READ,	/**< allow other processes to read the same file */
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (!m_FileHandle) {
		throw std::runtime_error("Failed to open file for reading.");
	}

	if (!GetFileSizeEx(m_FileHandle, &m_FileSize)) {
		CloseHandle(m_FileHandle);
		throw std::runtime_error("Failed to get file size.");
	}

	m_FileMapping = CreateFileMapping(m_FileHandle, 
		NULL,
		PAGE_READONLY,
		0, 
		0,
		NULL);

	if (!m_FileMapping) {
		CloseHandle(m_FileHandle);
		throw std::runtime_error("Failed to create a file mapping.");
	}

	m_Data = MapViewOfFile(m_FileMapping, 
		FILE_MAP_READ,
		0,
		0,
		0); /**< map entire file */
		
	if (!m_Data) {
		CloseHandle(m_FileMapping);
		CloseHandle(m_FileHandle);
		throw std::runtime_error("Failed to map file.");
	}
}

/**
 * \brief	Destructor, performs the required cleanup.
 */
MMapSource_impl::~MMapSource_impl()
{
	UnmapViewOfFile(m_Data);
	CloseHandle(m_FileMapping);
	CloseHandle(m_FileHandle);
}

bool MMapSource_impl::Read(void * a_Dst, size_t a_Count)
{
	uint64_t avail = GetSize() - FileOffset();
	if (avail < a_Count) {
		return false;
	}
	memcpy(a_Dst, ((const char *) m_Data) + m_Offset, a_Count);
	m_Offset += a_Count;
	return true;
}

bool MMapSource_impl::Skip(size_t a_Count)
{
	uint64_t avail = GetSize() - FileOffset();
	if (avail < a_Count) {
		return false;
	}
	m_Offset += a_Count;
	return true;
}

MMapSource::MMapSource(const wchar_t * a_FilePath)
{
	impl = new (std::nothrow) MMapSource_impl(a_FilePath);
}

MMapSource::MMapSource(const std::wstring & a_FilePath)
{
	impl = new (std::nothrow) MMapSource_impl(a_FilePath.c_str());
}
	
MMapSource::~MMapSource()
{
	delete impl;
}

bool MMapSource::end_of_file() const
{
	return impl->Eof();
}

bool MMapSource::read(void * a_Dst, size_t a_Count)
{
	return impl->Read(a_Dst, a_Count);
}

bool MMapSource::skip(size_t a_Count)
{
	return impl->Skip(a_Count);
}

}

}