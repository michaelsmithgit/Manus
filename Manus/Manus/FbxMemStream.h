#pragma once

#include <fbxsdk/core/fbxmanager.h>
#include <fbxsdk/core/fbxstream.h>

namespace fbxsdk
{
	class FbxMemStream :
		public FbxStream
	{
	private:
		char* m_data;
		FbxInt64 m_pos;
		FbxInt64 m_length;
		EState m_state;
		int m_error;

		int     m_reader_id;
		int     m_writer_id;

	public:
		FbxMemStream(FbxManager* pSdkManager, void* data, FbxInt64 length);
		virtual ~FbxMemStream();

		virtual EState GetState();
		virtual bool Open(void* pStreamData);
		virtual bool Close();
		virtual bool Flush();
		virtual int Write(const void* /*pData*/, int /*pSize*/);
		virtual int Read(void* /*pData*/, int /*pSize*/) const;
		virtual int GetReaderID() const;
		virtual int GetWriterID() const;
		virtual void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos);
		virtual long GetPosition() const;
		virtual void SetPosition(long pPosition);
		virtual int GetError() const;
		virtual void ClearError();
	};
}
