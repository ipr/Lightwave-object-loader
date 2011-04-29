// MemFile.h: interface for the CMemFile class.
//
// Implements file-IO related handling (platform-dependent).
// Currerntly only reading full file to buffer supported.
//
// TODO: reading stream from file as accessed by caller.
//
// TODO: unicode&ascii interface support?
//
// Ilkka Prusi 2006
//
//////////////////////////////////////////////////////////////////////

#ifndef _MEMFILE_H_
#define _MEMFILE_H_

//TODO: check these..
//needs VC2010 ?
//#include <stdint.h>


//#include <stdio.h> // NULL, FILE
//#include <malloc.h> // malloc(), free()

//#include <io.h> // _open(), _close()
//#include <fcntl.h> // _O_RDONLY
//#include <sys/stat.h> // _fstat()

//#include <sys/types.h> // _stat
//#include <stdlib.h> // _stat
//#include <string.h>

#include <iostream>
#include <string>
#include <fstream>
//#include <cstdio>

using namespace std;


class CMemFile  
{
private:
	FILE * m_pFile;
	void * m_pLWO_buf;
	unsigned long m_ulFilesize;
	string m_strFilename;

public:
	CMemFile(const char *file);
	virtual ~CMemFile();

protected:
	void Destroy();

	//unsigned long GetSizeOfFile();

public:

	const void *GetFileBuf() const
	{
		return m_pLWO_buf;
	};

	const char * GetFilename() const
	{
		return m_strFilename.c_str();
	};

	unsigned long GetFilesize() const
	{
		return m_ulFilesize;
	};

	bool LoadFile();

	const char *GetAtOffset(const unsigned long ulOffset, const unsigned long ulChunkSize);

};

#endif // ifndef _MEMFILE_H_
