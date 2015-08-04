#include "stdafx.h"
#include "FbxMemStream.h"
#include <fbxsdk.h>

#include <algorithm>

FbxMemStream::FbxMemStream(FbxManager* pSdkManager, void* data, FbxInt64 length)
	: m_data((char*)data)
	, m_pos(0)
	, m_length(length)
	, m_state(FbxStream::eClosed)
	, m_error(0)
{
	m_reader_id = pSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX (*.fbx)");
	m_writer_id = pSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX ascii (*.fbx)");
}


FbxMemStream::~FbxMemStream()
{
	Close();
}

FbxStream::EState FbxMemStream::GetState()
{
	return m_state;
}

bool FbxMemStream::Open(void* pStreamData)
{
	m_pos = 0;
	m_state = FbxStream::eOpen;
	return m_data != nullptr;
}

bool FbxMemStream::Close()
{
	m_state = FbxStream::eClosed;
	return true;
}

bool FbxMemStream::Flush()
{
	// Writes are not cached.
	return true;
}

int FbxMemStream::Write(const void* pData, int pSize)
{
	if (m_state == FbxStream::eClosed)
		return 0;

	size_t size = std::min((FbxInt64)pSize, m_pos);
	memcpy(m_data + m_pos, pData, size);

	m_pos += size;

	return size;
}

int FbxMemStream::Read(void* pData, int pSize) const
{
	if (m_state == FbxStream::eClosed)
		return 0;

	size_t size = std::min((FbxInt64)pSize, m_length - m_pos);
	memcpy(pData, m_data + m_pos, size);

	// We need to advance the position, but this function
	// was declared const, therefore we use a terrible
	// hack for which I offer my sincere apology.
	*(FbxInt64*)&m_pos += size;

	return size;
}

int FbxMemStream::GetReaderID() const
{
	return m_reader_id;
}

int FbxMemStream::GetWriterID() const
{
	return m_writer_id;
}

void FbxMemStream::Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos)
{
	if (pSeekPos == FbxFile::eBegin)
		m_pos = pOffset;
	else if (pSeekPos == FbxFile::eCurrent)
		m_pos += pOffset;
	else if (pSeekPos == FbxFile::eEnd)
		m_pos = m_length - pOffset;

	// Set the error if we're outside of bounds
	if (0 > m_pos || m_pos > m_length)
		m_error = 1;
}

long FbxMemStream::GetPosition() const
{
	return m_pos;
}

void FbxMemStream::SetPosition(long pPosition)
{
	m_pos = std::min((FbxInt64)pPosition, m_length);

	if (m_pos != pPosition)
		m_error = 1;
}

int FbxMemStream::GetError() const
{
	return m_error;
}

void FbxMemStream::ClearError()
{
	m_error = 0;
}
