// LwoReader.h: interface for the CLwoReader class.
//
// Use CMemFile to handle file-IO, this one retrieves the data from CMemFile.
// CLwoReader processes file-structure into list-form in RAM to make it
// easily accessible to whatever routines need the object-information.
//
// Ilkka Prusi 2006
//
//////////////////////////////////////////////////////////////////////

#ifndef _LWOREADER_H_
#define _LWOREADER_H_

//TODO: check these..
//needs VC2010 ?
//#include <stdint.h>


#include "LwoTags.h" // LWO tag-ID definitions

#include "MemFile.h" // file-IO handler
#include "LwoObjectData.h" // object information structure

#ifndef BYTE
typedef unsigned char      BYTE;
#endif

#ifndef WORD
typedef unsigned short      WORD;
#endif

// use 1-byte alignment when reading
// (instead of VC default)
#pragma pack(1)

// IFF chunk headers ("root" headers)
struct tChunkHeader
{
	unsigned int m_uiType;
	unsigned int m_uiSize;
};

// sub-chunk headers (below "root" headers")
struct tSubChunkHeader
{
	unsigned int m_uiType;
	unsigned short m_usSize;
};
#pragma pack()

// LWO2-IFF format file parsing
// to internal objects for easier handling
//
class CLwoReader  
{
private:

	// expected size of data according to file header
	unsigned int m_uiLWOSize;

	// ID-tag at header (LWO2/LWOB),
	// also LWLO which is LWOB+layers
	unsigned int m_uiLwoFileType;

	// processed data from filebuffer
	// (structured list for easier access)
	CLwoObjectData m_ObjectData;

protected:

	// tagi-id:n tekeminen char-stringistä
	// esim. FORM-stringistä
	inline unsigned int MakeTag(const char *buf);

	// byteswap bigendian<->little-endian
	// (2 bytes)
	inline unsigned short BSwap2s(const unsigned short *buf);

	// byteswap bigendian<->little-endian
	// (4 bytes)
	inline unsigned int BSwap4i(const unsigned int *buf);

	// byteswap for float:
	// avoid int<->float cast during byteswap to avoid rounding errors
	inline float BSwapF(const float fVal);

	// TODO:?
	/*
	float RadToDeg(const float fRad);
	float DegToRad(const float fDeg);
	*/

	// get variable-length index-value from chunk-position,
	// may be 2 or 4 byte integer
	inline unsigned int GetVarlenIX(const char *pChunk, int &iIxSize);

	// get string and length where possible padding may occur
	inline string GetPaddedString(const char *pBufPos, unsigned int &uiSize);

	// handle IFF-header and check file type (LWOB/LWO2)
	bool HandleFileHeader(const char *pLwoBuf, const unsigned long ulFileSize);

	// primary-chunk type
	unsigned int GetChunkType(const char *pLwoBuf, unsigned int &uiChunkSize);

	// sub-chunk type (e.g. ID_SURF)
	unsigned int GetSubChunkType(const char *pLwoBuf, unsigned short &usChunkSize);

	// TODO: checking if there's padding byte after sub-chunk
	// so we can start next sub-chunk correctly..
	//bool IsSubchunkPadded(const char *pLwoBuf, const unsigned short usSubCSize, const unsigned int uiChunkSize);

	bool ProcessChunk(const char *pChunk, const unsigned int uiChunkType, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_TAGS(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_PTAG(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_LAYR(const char *pChunk, const unsigned int uiChunkSize);
	bool Handle_LWLO_ID_LAYR(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_BBOX(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_POLS(const char *pChunk, const unsigned int uiChunkSize);
	bool Handle_LWOB_ID_POLS(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWOB_ID_CRVS(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWOB_ID_SRFS(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_SURF(const char *pChunk, const unsigned int uiChunkSize);
	bool Handle_LWOB_ID_SURF(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_ENVL(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_CLIP(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_VMAP(const char *pChunk, const unsigned int uiChunkSize);

	bool Handle_LWO2_ID_VMAD(const char *pChunk, const unsigned int uiChunkSize);

	// TODO: sub-chunk handling separately?
	//bool Handle_LWO2_SubChunk(const unsigned int uiType, const unsigned int uiParentType, const char *pSChunk, const unsigned short usChunkSize);
	//bool Handle_LWO2_ID_SURF_BLOK(const char *pChunk, const unsigned int uiChunkSize);

	// this is testing
	char *Handle_LWO2_BLOK(char *pBufPos, const unsigned short usBLOKSize);
	char *Handle_LWO2_BLOK_Chunks(char *pBufPos);

public:
	CLwoReader();
	virtual ~CLwoReader();

	bool ProcessFromFile(CMemFile &LwoFile);

};

#endif // ifndef _LWOREADER_H_

