//////////////////////////////////////////////////////////////////////
// MemFile.cpp: implementation of the CMemFile class.
//
// Loads file to memory - just a way to hide the 
// loading and allocating memory
//
// Ilkka Prusi 2006
//
//////////////////////////////////////////////////////////////////////

#include "MemFile.h"

// need explicit include for GCC..
#include <cstdlib>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemFile::CMemFile(const char *file)
: m_strFilename(file)
, m_pLWO_buf(NULL)
, m_pFile(NULL)
{
}

CMemFile::~CMemFile()
{
	Destroy();
}

void CMemFile::Destroy()
{
	if (m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

	if (m_pLWO_buf != NULL)
	{
		free(m_pLWO_buf);
		m_pLWO_buf = NULL;
	}
	m_ulFilesize = 0;
}

/*
unsigned long CMemFile::GetSizeOfFile()
{
	struct _stat sStatbuf;
	int sfh, sres;
	sfh = _open(argv[1], _O_RDONLY);
	sres = _fstat(sfh, &sStatbuf);
	_close( sfh );
	return sStatbuf.st_size;
}
*/


bool CMemFile::LoadFile()
{
	if (m_pFile != NULL
		|| m_pLWO_buf != NULL)
	{
		// ignore read second time
		return false;
	}

	// opening the file, "readonly, binary"
	//errno_t err = fopen_s(&m_pFile, m_strFilename.c_str(),"rb");
	m_pFile = fopen(m_strFilename.c_str(),"rb");
	if (m_pFile == NULL)
	{
		return false;
	}

	// obtain file size (this method is normal)
	fseek(m_pFile, 0, SEEK_END);
	long lFilesize = ftell(m_pFile);
	rewind(m_pFile);

	// ftell returns -1 on error,
	// also empty file is not useful
	if (lFilesize <= 0)
	{
		return false;
	}

	// allocate memory, the size of the real file
	m_pLWO_buf = malloc( lFilesize );
	if (m_pLWO_buf == NULL)
	{
		// not enough ram?
		// -> exit
		return false;
	}

	memset(m_pLWO_buf, 0, lFilesize);

	long lReadTotal = 0;
	while (lReadTotal < lFilesize)
	{
		const int iBlobCount = 1;
		size_t iRead = 0;
		iRead = fread( m_pLWO_buf, (lFilesize-lReadTotal), iBlobCount, m_pFile );

		// if reading failed, we exit
		if (iRead == 0)
		{
			return false;
		}

		// normally when we read as single blob,
		// return value is also count of blobs read (1)
		if (iRead == 1
			&& iBlobCount == 1)
		{
			// count amount (in bytes) read
			// when reading single blob (for checking
			// and support other ways later)
			lReadTotal += (lFilesize-lReadTotal);
		}
		else
		{
			// variable count of blobs
			// not supported yet
			return false;
		}
	}

	// now we have data in the buffer
	m_ulFilesize = lReadTotal;

	if (m_pFile != NULL)
	{
		// we may close the file
		fclose(m_pFile);
		m_pFile = NULL;
	}
	return true;
}

// TODO: in case of streaming from file, determine amount to read
// and get that amount.
//
// for now, just return from full-file buffer read previously.
// 
const char *CMemFile::GetAtOffset(unsigned long ulOffset, const unsigned long ulChunkSize)
{
	// outside of current buffer
	if (ulOffset >= m_ulFilesize)
	{
		return NULL;
	}
	if ((ulChunkSize+ulOffset) > m_ulFilesize)
	{
		// attempt to read too much ?
		return NULL;
	}

	char *pBuf = (char*)GetFileBuf();

	return (&pBuf[ulOffset]);
}

