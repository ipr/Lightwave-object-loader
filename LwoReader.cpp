// LwoReader.cpp: implementation of the CLwoReader class.
//
// Use CMemFile to handle file-IO, this one retrieves the data from CMemFile.
// CLwoReader processes file-structure into list-form in RAM to make it
// easily accessible to whatever routines need the object-information.
//
// Ilkka Prusi 2006
//
//////////////////////////////////////////////////////////////////////

#include "LwoReader.h"


/////// protected methods

// make tag-ID from file
unsigned int CLwoReader::MakeTag(const char *buf)
{
	return (
		(unsigned long) (buf[0])<<24 
		| (unsigned long) (buf[1])<<16 
		| (unsigned long) (buf[2])<<8 
		| (unsigned long) (buf[3]));
}

// byteswap 2 (short)
unsigned short CLwoReader::BSwap2s(const unsigned short *buf)
{
	unsigned short tmp = (*buf);
	return (((tmp >> 8)) | (tmp << 8));
}

// byteswap 4 (int)
unsigned int CLwoReader::BSwap4i(const unsigned int *buf)
{
	unsigned int tmp = (*buf);
	return (
			((tmp & 0x000000FF) << 24) + ((tmp & 0x0000FF00) <<8) +
			((tmp & 0x00FF0000) >> 8) + ((tmp & 0xFF000000) >>24)
			);
}

// byteswap 4: float special case (see comments)
float CLwoReader::BSwapF(const float fVal)
{
	float fTmp = fVal;

	// cast the "bit-array" via pointer for byteswap
	// so we don't make int<->float cast by mistake

	unsigned int uiTmp = BSwap4i((unsigned int*)(&fTmp));

	fTmp = (*((float*)(&uiTmp)));
	return fTmp;
}

unsigned int CLwoReader::GetVarlenIX(const char *pChunk, int &iIxSize)
{
	// temp for value from buffer
	unsigned int uiVertIndex = 0;

	// get variable-length index-value,
	// it may be 2 or 4 bytes:
	// if first byte is 0xFF -> 4-byte index,
	// otherwise 2-byte index
	if (pChunk[0] == 0xFF)
	{
		// get four bytes and 
		// mask out the first byte
		uiVertIndex = BSwap4i((unsigned int*)pChunk);

		// mask out upper-most byte
		uiVertIndex = (uiVertIndex & 0x00FFFFFF);

		// out: 4-byte int found
		iIxSize = 4;
	}
	else
	{
		// use the two bytes,
		// keep as four-byte integer
		// for simplicity and efficiency
		uiVertIndex = (int)BSwap2s((unsigned short*)pChunk);

		// out: 2-byte int found
		iIxSize = 2;
	}
	return uiVertIndex;
}

string CLwoReader::GetPaddedString(const char *pBufPos, unsigned int &uiSize)
{
	// each string is ASCII with terminating NULL,
	// but can have double-NULL to make even-byte alignment
	string szName = pBufPos;
	uiSize = szName.length();

	if (uiSize % 2 == 0)
	{
		// remainder zero: even-length string
		// -> handle padding-byte (second terminating NULL)
		uiSize += 2;
	}
	else
	{
		// non-zero remainder: odd-length string
		// -> only single terminating NULL
		uiSize += 1;
	}
	return szName;
}

bool CLwoReader::HandleFileHeader(const char *pLwoBuf, const unsigned long ulFileSize)
{
	if (pLwoBuf == NULL
		|| ulFileSize < 12)
	{
		// invalid buffer or not enough data
		return false;
	}

	char *pBufPos = (char*)pLwoBuf;
	unsigned int uiType = 0; // bufferi chunk-tyyppitagille

	// read IFF-header tag: should be FORM
	uiType = MakeTag(pBufPos); // tehdään tagi chunkin headerista (ekat neljä byteä bufferissa)
	if (uiType != ID_FORM)
	{
		// missing IFF-header/unsupport format
		// -> abort
		return false;
	}

	// siirry chunkin kokoon
	pBufPos = (pBufPos +4);

	// chunkin koko: filukoko-8 (348->340)
	// LWO2:ssa luvut aina bigendianisessa muodossa
	// byteswap bigendian->little-endian
	unsigned int uiLWOSize = 0;
	uiLWOSize = BSwap4i((unsigned int*)pBufPos);
	if (uiLWOSize <= 0)
	{
		return false;
	}
	if (uiLWOSize != (ulFileSize-8))
	{
		// invalid chunk length
		return false;
	}

	// move to next part and read tag
	pBufPos = (pBufPos +4);

	// file-type tag should be here (LWO2, LWOB, LWLO)
	m_uiLwoFileType = MakeTag(pBufPos);

	// LWO2: newer, since LW6.0 (layers, envelopes etc.)
	// LWOB: older, pre-LW6.0 (no layers)
	// LWLO: older, pre-LW6.0 (with layers)
	if (m_uiLwoFileType != ID_LWO2
		&& m_uiLwoFileType != ID_LWOB
		&& m_uiLwoFileType != ID_LWLO)
	{
		return false;
	}

	// keep this expected length according to file header
	m_uiLWOSize = uiLWOSize;
	return true;
}

unsigned int CLwoReader::GetChunkType(const char *pLwoBuf, unsigned int &uiChunkSize)
{
	char *pBufPos = (char*)pLwoBuf;
	unsigned int uiType = MakeTag(pBufPos);

	// pos of chunk size
	pBufPos = (pBufPos +4);

	uiChunkSize = BSwap4i((unsigned int*)pBufPos);

	return uiType;
}

// for sub-chunk used in e.g. ID_SURF (surface)
unsigned int CLwoReader::GetSubChunkType(const char *pLwoBuf, unsigned short &usChunkSize)
{
	char *pBufPos = (char*)pLwoBuf;
	unsigned int uiType = MakeTag(pBufPos);

	// pos of chunk size
	pBufPos = (pBufPos +4);

	usChunkSize = BSwap2s((unsigned short*)pBufPos);

	return uiType;
}

bool CLwoReader::ProcessChunk(const char *pChunk, const unsigned int uiChunkType, const unsigned int uiChunkSize)
{
	if (pChunk == NULL
		|| uiChunkSize <= 0)
	{
		return false;
	}

	char *pBufPos = (char*)pChunk;

	/*
	TODO: use this check before all chunk-types
	to verify we create internally a layer even if file doesn't specify one..?

	// locate the current layer
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	// older format may have chunks and no layer?
	// -> may be in some other cases also that we don't have layer yet..
	if (pCurrentLayer == NULL
		&& uiChunkType != ID_LAYR)
	{
		pCurrentLayer = new CLwoLayer(0);
		m_ObjectData.AddChunk(pCurrentLayer);
	}
	*/

	// check type and handle chunk
	switch (uiChunkType)
	{
	case ID_TAGS:
		// this chunk should have tag name(s),
		// can be array of tag names:
		// this does not belong to a layer
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_TAGS(pChunk, uiChunkSize);
		}
		break;

	case ID_PTAG:
		// associates tags with polygons
		// e.g. surface -> polygon
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_PTAG(pChunk, uiChunkSize);
		}
		break;

	case ID_LAYR:
		// layer: combines set of geometry
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_LAYR(pChunk, uiChunkSize);
		}
		else if (m_uiLwoFileType == ID_LWLO)
		{
			return Handle_LWLO_ID_LAYR(pChunk, uiChunkSize);
		}
		break;

	case ID_BBOX:
		// bounding box: extents of layer
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_BBOX(pChunk, uiChunkSize);
		}
		break;

	case ID_PNTS:
		// points:
		// vertex coordinates (triplets of floats),
		// not yet polygons (need poly-index list for that).
		// this should be same in LWO2 and LWOB (also LWLO)?
		{
			// locate the current layer
			CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

			// older format may have chunks and no layer?
			// -> may be in some other cases also that we don't have layer yet..
			if (pCurrentLayer == NULL)
			{
				pCurrentLayer = new CLwoLayer(0);
				m_ObjectData.AddChunk(pCurrentLayer);
			}

			// points is the raw-coordinate position data
			// which form polygons with indices and surfaces
			CLwoPoints *pPoints = new CLwoPoints(pCurrentLayer->m_uiLayerIndex);

			pPoints->m_lValueCount = uiChunkSize/sizeof(float);
			pPoints->m_pfPointList = new float[pPoints->m_lValueCount];

			float *pfPointBuf = (float*)pBufPos;
			for (long i = 0; i < pPoints->m_lValueCount; i++)
			{
				// byteswap and keep in box-object
				pPoints->m_pfPointList[i] = BSwapF(pfPointBuf[i]);
			}

			// keep reference in layer, store to object data container
			pCurrentLayer->AddChunkToLayer(pPoints);
			m_ObjectData.AddChunk(pPoints);
		}
		break;

	case ID_POLS:
		// polygons, these refer by index to most-recent point-list
		// to define which are the vertices of each polygon
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_POLS(pChunk, uiChunkSize);
		}
		else if (m_uiLwoFileType == ID_LWOB
			|| m_uiLwoFileType == ID_LWLO)
		{
			// this older format has some differences..
			// warning: not fixed yet?
			return Handle_LWOB_ID_POLS(pChunk, uiChunkSize);
		}
		break;

	case ID_CRVS:
		// this it's own chunk in LWOB
		// instead of sub-type of POLS (as in LWO2)
		if (m_uiLwoFileType == ID_LWOB
			|| m_uiLwoFileType == ID_LWLO)
		{
			return Handle_LWOB_ID_CRVS(pChunk, uiChunkSize);
		}
		break;

	case ID_SRFS:
		// LWOB (pre-6.0) only list of surfaces?
		// (names of surfaces, mapping-by-name)
		if (m_uiLwoFileType == ID_LWOB
			|| m_uiLwoFileType == ID_LWLO)
		{
			return Handle_LWOB_ID_SRFS(pChunk, uiChunkSize);
		}
		break;

	case ID_SURF:
		// surface-defition refers to polygons,
		// describes colors, texturing etc.
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_SURF(pChunk, uiChunkSize);
		}
		else if (m_uiLwoFileType == ID_LWOB
			|| m_uiLwoFileType == ID_LWLO)
		{
			return Handle_LWOB_ID_SURF(pChunk, uiChunkSize);
		}
		break;

	case ID_ENVL:
		// envelope with sub-chunks
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_ENVL(pChunk, uiChunkSize);
		}
		break;

	case ID_CLIP:
		// clip with sub-chunks:
		// single, possibly time-varying image
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_CLIP(pChunk, uiChunkSize);
		}
		break;

	case ID_VMAP:
		// vector-map, refers to most-recent points-list
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_VMAP(pChunk, uiChunkSize);
		}
		break;

	case ID_VMAD:
		// LW6.5: Discontinuous Vertex Mapping
		if (m_uiLwoFileType == ID_LWO2)
		{
			return Handle_LWO2_ID_VMAD(pChunk, uiChunkSize);
		}
		break;

	case ID_DESC:
		// description line
		{
			unsigned int uiStrLen = 0;
			string szFileDescription = GetPaddedString(pBufPos, uiStrLen);
			pBufPos = (pBufPos + uiStrLen);
		}
		break;

	case ID_TEXT:
		// comments about anything in the file
		{
			unsigned int uiStrLen = 0;
			string szComments = GetPaddedString(pBufPos, uiStrLen);
			pBufPos = (pBufPos + uiStrLen);
		}
		break;

	case ID_ICON:
		// thumbnail icon image
		break;
	}

	return true;
}

bool CLwoReader::Handle_LWO2_ID_TAGS(const char *pChunk, const unsigned int uiChunkSize)
{
	char *pBufPos = (char*)pChunk;

	// tags: list of names referred to with 0-based index

	CLwoTagnameList *pTagnames = new CLwoTagnameList();

	char *pTagsEnd = (pBufPos + uiChunkSize);
	while (pBufPos != pTagsEnd)
	{
		// size with possible padding
		unsigned int uiStrLen = 0;

		// each string is ASCII with terminating NULL,
		// but can have double-NULL to make even-byte alignment
		string szTagName = GetPaddedString(pBufPos, uiStrLen);

		// keep name
		pTagnames->AddTagname(szTagName);

		// more strings? (this should be in loop then)
		pBufPos = (pBufPos + uiStrLen);
	}

	// keep tag-name list
	m_ObjectData.AddChunk(pTagnames);
	return true;
}

bool CLwoReader::Handle_LWO2_ID_PTAG(const char *pChunk, const unsigned int uiChunkSize)
{
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	CLwoPolygons *pPrevPols = (CLwoPolygons*)m_ObjectData.GetPreviousOfType(ID_POLS);
	CLwoChunk *pTags = m_ObjectData.GetPreviousOfType(ID_TAGS);

	char *pBufPos = (char*)pChunk;

	CLwoPolyTags *pPolyTags = new CLwoPolyTags(pCurrentLayer->m_uiLayerIndex);

	// keep reference to latest polygon-list
	// (should reverse this relation?)
	pPolyTags->m_pPolyList = pPrevPols;

	// SURF, PART, SMGP
	pPolyTags->m_uiPtagTypeID = MakeTag(pBufPos);

	// count end
	char *pEnd = (pBufPos + uiChunkSize);

	// skip to data
	pBufPos = (pBufPos +4);

	while (pBufPos != pEnd)
	{
		// here we have pair<polIX, tagIX>:
		// polygon-index is variable-length and tag index is 2 bytes

		// reading index determines if it's 2 or 4 bytes
		int iIxSize = 0;

		// get polygon index (var-len), 0-based
		int iPolIX = (int)GetVarlenIX(pBufPos, iIxSize);

		// skip to past index
		pBufPos = (pBufPos +iIxSize);

		// get tag index, 0-based
		int iTagIX = (int)BSwap2s((unsigned short *)pBufPos);

		// skip to next "row" (pair)
		pBufPos = (pBufPos +2);

		// keep mapping
		pPolyTags->m_PolyTagList.push_back(CLwoPolyTags::tPolToTag(iPolIX, iTagIX));
	}

	// keep reference in layer, store to chunk-list
	pCurrentLayer->AddChunkToLayer(pPolyTags);
	m_ObjectData.AddChunk(pPolyTags);
	return true;
}

// for newer LWO2-layer definition
bool CLwoReader::Handle_LWO2_ID_LAYR(const char *pChunk, const unsigned int uiChunkSize)
{
	// layer: all chunks from this upto next layer
	// belong to this layer

	// TODO: locate previous/parent layer (if any?)
	//CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	CLwoLayer *pLayer = new CLwoLayer(m_ObjectData.GetNextLayerIndex());

	char *pBufPos = (char*)pChunk;

	// count chunk end for later..
	char *pEnd = (pBufPos + uiChunkSize);

	// layer-index (0-based)
	pLayer->m_usLayerNumber = BSwap2s((unsigned short *)pBufPos);
	pBufPos = (pBufPos +2);

	pLayer->m_usLayerFlags = BSwap2s((unsigned short *)pBufPos);
	pBufPos = (pBufPos +2);

	// layer pivot-point (vertex, triplet of floats)
	pLayer->m_pfPivotPoint = new float[3];

	float *pfPointBuf = (float*)pBufPos;
	for (long i = 0; i < 3; i++)
	{
		// byteswap and keep in container
		pLayer->m_pfPivotPoint[i] = BSwapF(pfPointBuf[i]);
	}
	pBufPos = (pBufPos + 3*sizeof(float));

	// get string, check size (in case of padding)
	unsigned int uiSize = 0;
	pLayer->m_szLayerName = GetPaddedString(pBufPos, uiSize);
	pBufPos = (pBufPos + uiSize);

	// now if we are not yet at end there should be U2 for parent-number,
	// which can be missing if no parent
	if (pBufPos != pEnd)
	{
		pLayer->m_iParentLayerIndex = (int)BSwap2s((unsigned short *)pBufPos);
	}

	m_ObjectData.AddChunk(pLayer);
	return true;
}

// for older LWLO-format layer definition
bool CLwoReader::Handle_LWLO_ID_LAYR(const char *pChunk, const unsigned int uiChunkSize)
{
	// TODO: locate previous layer (if any?)
	//CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	CLwoLayer *pLayer = new CLwoLayer(m_ObjectData.GetNextLayerIndex());

	char *pBufPos = (char*)pChunk;

	// layer number: should be from 1..10 in LWLO
	pLayer->m_usLayerNumber = BSwap2s((unsigned short *)pBufPos);
	pBufPos = (pBufPos +2);

	// lowest-order bit defined: 1 if active layer, 0 if background
	pLayer->m_usLayerFlags = BSwap2s((unsigned short *)pBufPos);
	pBufPos = (pBufPos +2);

	// get string (name of layer) 
	// and check size (in case of padding for even-address)
	unsigned int uiSize = 0;
	pLayer->m_szLayerName = GetPaddedString(pBufPos, uiSize);
	pBufPos = (pBufPos + uiSize);

	m_ObjectData.AddChunk(pLayer);
	return true;
}

bool CLwoReader::Handle_LWO2_ID_BBOX(const char *pChunk, const unsigned int uiChunkSize)
{
	// locate the current layer
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);
	CLwoBoundingBox *pBBox = new CLwoBoundingBox(pCurrentLayer->m_uiLayerIndex);

	char *pBufPos = (char*)pChunk;

	pBBox->m_lValueCount = uiChunkSize/sizeof(float);
	pBBox->m_pfBoxExtents = new float[pBBox->m_lValueCount];

	float *pfPointBuf = (float*)pBufPos;
	for (long i = 0; i < pBBox->m_lValueCount; i++)
	{
		// byteswap and keep in box-object
		pBBox->m_pfBoxExtents[i] = BSwapF(pfPointBuf[i]);
	}

	pCurrentLayer->AddChunkToLayer(pBBox); // keep box-reference in layer for fast access
	m_ObjectData.AddChunk(pBBox); // actual storage of the container
	return true;
}

// newer LWO2 polygons-chunk
bool CLwoReader::Handle_LWO2_ID_POLS(const char *pChunk, const unsigned int uiChunkSize)
{
	// polygons, these refer by index to most-recent point-list
	// to define which are the vertices of each polygon
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	CLwoPolygons *pPolyList = new CLwoPolygons(pCurrentLayer->m_uiLayerIndex);
	pPolyList->m_pPointsList = (CLwoPoints*)m_ObjectData.GetPreviousOfType(ID_PNTS);

	char *pBufPos = (char*)pChunk;

	// count end of chunk for handling
	char *pPolyEnd = (pBufPos + uiChunkSize);

	// FACE, CURV, PTCH, MBAL or BONE
	// Note! we need to mask out upper 6-bits of each vertex-count
	// when POLS=CURV !!
	//
	// keep the sub-chunk information
	pPolyList->m_uiPolyTypeID = MakeTag(pBufPos);

	// skip sub-type to actual data
	pBufPos = (pBufPos +4);

	// TODO: we might want to pre-process points into polygons
	// here for simplicity later when actually using the data?

	long lPolyRowIndex = 0;
	while (pBufPos != pPolyEnd)
	{
		// pBufPos has vertex-count
		unsigned short wVertexCount = BSwap2s((unsigned short*)pBufPos);

		// get and mask out flags in upper 6-bits
		unsigned short wFlags = ((0xfc00 & wVertexCount) >> 10);
		wVertexCount = (0x03ff & wVertexCount);

		// skip past vertex-count (short)
		// to actual index-list
		pBufPos = (pBufPos +2);

		// only CURV uses flags there but need to mask out anyway,
		// this is sub-definition in POLS-chunk in LWO2.
		// note: LW9 
		if (pPolyList->m_uiPolyTypeID == ID_CURV)
		{
			// two lower flags are continuity point toggles,
			// remaining 4 are additional vertex count bits
			// TODO: mask&handle rest
			unsigned short wAddVCount = (wFlags & 0x3FC);
			wAddVCount >>= 2; // shift additional count also?
			//wVertexCount += wAddVCount; // is this correct now?
			wFlags = (wFlags & 0x3); // only two flags should remain?
		}

		CLwoPolygons::CLwoPolyRow *pVertList = new CLwoPolygons::CLwoPolyRow(lPolyRowIndex, wVertexCount, wFlags);

		// handle each index according to size
		for (int l = 0; l < wVertexCount; l++)
		{
			// reading index determines if it's 2 or 4 bytes
			int iIxSize = 0;

			// get vertex index from position,
			// get size of it for locating next (may be 2 or 4 bytes)
			// and keep this index in list
			pVertList->m_piIndices[l] = (int)GetVarlenIX(pBufPos, iIxSize);

			// skip to next index
			pBufPos = (pBufPos +iIxSize);
		} // for

		// keep row for later
		pPolyList->m_PolyList.push_back(pVertList);
		lPolyRowIndex++;
	}

	pCurrentLayer->AddChunkToLayer(pPolyList);
	m_ObjectData.AddChunk(pPolyList);
	return true;
}

// older LWOB-format polygons-chunk
bool CLwoReader::Handle_LWOB_ID_POLS(const char *pChunk, const unsigned int uiChunkSize)
{
	// polygons, these refer by index to most-recent point-list
	// to define which are the vertices of each polygon
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	if (pCurrentLayer == NULL)
	{
		pCurrentLayer = new CLwoLayer(0);
		m_ObjectData.AddChunk(pCurrentLayer);
	}

	CLwoPolygons *pPolyList = new CLwoPolygons(pCurrentLayer->m_uiLayerIndex);
	pPolyList->m_pPointsList = (CLwoPoints*)m_ObjectData.GetPreviousOfType(ID_PNTS);

	char *pBufPos = (char*)pChunk;

	// count end of chunk for handling
	char *pPolyEnd = (pBufPos + uiChunkSize);

	// in older format, there is not tag in current pos
	// -> assume FACE always here (LWOB)
	pPolyList->m_uiPolyTypeID = ID_FACE;


	// TODO: we might want to pre-process points into polygons
	// here for simplicity later when actually using the data?


	long lPolyRowIndex = 0;
	while (pBufPos != pPolyEnd)
	{
		// pBufPos has vertex-count
		unsigned short wVertexCount = BSwap2s((unsigned short*)pBufPos);

		// get and mask out flags in upper 6-bits
		unsigned short wFlags = ((0xfc00 & wVertexCount) >> 10);
		wVertexCount = (0x03ff & wVertexCount);

		// only CURV uses flags there, but only for ID_CURV
		// which is actually it's own chunk-type in LWOB
		// instead of sub-defition of POLS (as in LWO2)
		//if (pPolyList->m_uiPolyTypeID == ID_CURV)

		// skip past vertex-count (short)
		// to actual index-list
		pBufPos = (pBufPos +2);

		CLwoPolygons::CLwoPolyRow *pVertList = new CLwoPolygons::CLwoPolyRow(lPolyRowIndex, wVertexCount, wFlags);

		// handle each index according to size
		for (int l = 0; l < wVertexCount; l++)
		{
			// in older LWOB, we have only 2-byte integers and indices?
			// (newer have variable-length indices)
			pVertList->m_piIndices[l] = BSwap2s((unsigned short*)pBufPos);

			// skip to next index
			pBufPos = (pBufPos +2);
		} // for

		// additionally, index to surface-list after each vertex-index "row"?
		short sSurfaceIndex = BSwap2s((unsigned short*)pBufPos);
		pBufPos = (pBufPos +2);

		if (sSurfaceIndex < 0)
		{
			// we keep the absolute-number of negative surface-index 
			// to determine actual surface-index
			sSurfaceIndex = abs(sSurfaceIndex);
			// -> keep this with actual poly
			pVertList->m_wSurfaceIndex = sSurfaceIndex;

			// if negative (has detail-polygon), 
			// followed by U2 to define how many detail-polygons
			// belong to current polygon and list of those detail-polygons:
			// detail-polygons cannot have sub-details but otherwise same as normal

			// detail-poly count
			unsigned short usDPolyCount = BSwap2s((unsigned short*)pBufPos);
			pBufPos = (pBufPos +2);

			// count of vertex-indices for the poly
			unsigned short usDPolyVertCount = BSwap2s((unsigned short*)pBufPos);
			pBufPos = (pBufPos +2);

			for (int v = 0; v < usDPolyVertCount; v++)
			{
				// in older LWOB, we have only 2-byte integers and indices?
				// (newer have variable-length indices)
				//pVertList->m_piIndices[l] = BSwap2s((unsigned short*)pBufPos);
				unsigned short usTemp = BSwap2s((unsigned short*)pBufPos);

				// skip to next index
				pBufPos = (pBufPos +2);

			}

			// surface-index of the detail-polygon
			short sDSurfaceIndex = BSwap2s((unsigned short*)pBufPos);
			pBufPos = (pBufPos +2);
		}
		else
		{
			pVertList->m_wSurfaceIndex = sSurfaceIndex;
		}

		// keep row for later
		pPolyList->m_PolyList.push_back(pVertList);
		lPolyRowIndex++;
	}

	pCurrentLayer->AddChunkToLayer(pPolyList);
	m_ObjectData.AddChunk(pPolyList);
	return true;
}

// spline curve data
bool CLwoReader::Handle_LWOB_ID_CRVS(const char *pChunk, const unsigned int uiChunkSize)
{
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	if (pCurrentLayer == NULL)
	{
		pCurrentLayer = new CLwoLayer(0);
		m_ObjectData.AddChunk(pCurrentLayer);
	}

	// use polylist as curve (like with LWO2 sub-type)
	CLwoPolygons *pPolyList = new CLwoPolygons(pCurrentLayer->m_uiLayerIndex);
	pPolyList->m_pPointsList = (CLwoPoints*)m_ObjectData.GetPreviousOfType(ID_PNTS);

	// assume CURV as type here (sub-type of polygons in LWO2)
	pPolyList->m_uiPolyTypeID = ID_CURV;

	char *pBufPos = (char*)pChunk;

	// count end of chunk for handling
	const char *pEnd = (pBufPos + uiChunkSize);

	long lPolyRowIndex = 0;
	while (pBufPos != pEnd)
	{
		unsigned short wVertexCount = BSwap2s((unsigned short*)pBufPos);
		pBufPos = (pBufPos +2);

		CLwoPolygons::CLwoPolyRow *pVertList = new CLwoPolygons::CLwoPolyRow(lPolyRowIndex, wVertexCount, 0);

		for (int i = 0; i < wVertexCount; i++)
		{
			// indices like in POLS but cannot have detail polygons
			// (TODO: was this 1 or 0 based?)
			pVertList->m_piIndices[i] = (int)BSwap2s((unsigned short*)pBufPos);

			// skip to next
			pBufPos = (pBufPos +2);
		}

		// indices here like in POLS but cannot have detail polygons
		pVertList->m_wSurfaceIndex = BSwap2s((unsigned short*)pBufPos);
		pBufPos = (pBufPos +2);

		// flags: if bit zero is set then the first point is a continuity
		// control point, and if bit one is set then the last point is
		pVertList->m_wFlags = BSwap2s((unsigned short*)pBufPos);
		pBufPos = (pBufPos +2);
		
		// keep row for later
		pPolyList->m_PolyList.push_back(pVertList);
		lPolyRowIndex++;
	}

	pCurrentLayer->AddChunkToLayer(pPolyList);
	m_ObjectData.AddChunk(pPolyList);
	return true;
}

bool CLwoReader::Handle_LWOB_ID_SRFS(const char *pChunk, const unsigned int uiChunkSize)
{
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	if (pCurrentLayer == NULL)
	{
		pCurrentLayer = new CLwoLayer(0);
		m_ObjectData.AddChunk(pCurrentLayer);
	}

	 // 1-based index on older LWOB, handle internally as 0-based?
	long lSrfsIx = 0;
	char *pBufPos = (char*)pChunk;
	char *pSurfacesEnd = (pBufPos + uiChunkSize);
	while (pBufPos != pSurfacesEnd)
	{
		// size with possible padding
		unsigned int uiStrLen = 0;

		// note: in LWOB-format there are 1-based indices
		// to this surface-list where most other indices are zero-based?

		// each string is ASCII with terminating NULL,
		// but can have double-NULL to make even-byte alignment
		string szSurfaceName = GetPaddedString(pBufPos, uiStrLen);

		// TODO: keep the names somewhere..

		// more strings? (this should be in loop then)
		pBufPos = (pBufPos + uiStrLen);
		lSrfsIx++;
	}
	return true;
}

bool CLwoReader::Handle_LWO2_ID_SURF(const char *pChunk, const unsigned int uiChunkSize)
{
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	// in LWO2, surface is linked to polygon via PTAG-list
	// of mapping between polygons and surfaces (0-based)
	CLwoSurface *pSurfaces = new CLwoSurface(pCurrentLayer->m_uiLayerIndex);

	char *pBufPos = (char*)pChunk;

	// count end of chunk for handling
	const char *pSurfEnd = (pBufPos + uiChunkSize);

	unsigned int uiStrSize = 0;

	// name of this surface
	pSurfaces->m_szSurfaceName = GetPaddedString(pBufPos, uiStrSize);
	pBufPos = (pBufPos + uiStrSize);

	// LWO2 can have parent-surface name:
	// name of parent-surface (if any),
	// a surface can inherit from it's parent
	pSurfaces->m_szParentSurfaceName = GetPaddedString(pBufPos, uiStrSize);
	pBufPos = (pBufPos + uiStrSize);

	while (pBufPos != pSurfEnd)
	{
		unsigned short usSCSize = 0;
		unsigned int uiSCType = GetSubChunkType(pBufPos, usSCSize);
		pBufPos = (pBufPos +6); // type (4) + size (2) bytes

		// count end of sub-chunk
		const char *pSubChunkEnd = (pBufPos + usSCSize);

		// TODO: need to move BLOK separately..
		// from here..

		// blok: nested sub-chunks
		if (uiSCType == ID_BLOK)
		{
			// skip this sub-chunk
			pBufPos = (pBufPos +usSCSize);

			// processing
			//pBufPos = Handle_LWO2_BLOK(pBufPos, usSCSize);

			continue;
		}

		// for basic surface parameters
		switch (uiSCType)
		{
		case ID_COLR:
			{
				// three color-values (RGB) in 4-byte floats,
				// and one VX for enveloping
				float *pfRGB = new float[3];
				float *pfChunkRGB = (float*)pBufPos;
				for (int i = 0; i < 3; i++)
				{
					pfRGB[i] = BSwapF(pfChunkRGB[i]);
				}
				pBufPos = (pBufPos + 3*sizeof(float));

				// TODO: keep buffer instead of deleting
				delete pfRGB;

				// also index to envelope
				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_DIFF:
		case ID_LUMI:
		case ID_SPEC:
		case ID_REFL:
		case ID_TRAN:
		case ID_TRNL:
			//DIFF, LUMI, SPEC, REFL, TRAN, TRNL
			// have same structure of information,
			// if any of these is missing, value of zero is assumed for it
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				// also index to envelope
				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_GLOS:
			// glossiness
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				// also index to envelope
				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_SHRP:
			// sharpness
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				// also index to envelope
				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_BUMP:
			// bump intensity
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				// also index to envelope
				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_SIDE:
			// polygon sidedness
			{
				unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_SMAN:
			// max smoothing angle (in radians)
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);
			}
			break;

		case ID_RFOP:
			// reflection-options
			{
				unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_RIMG:
			// reflection map image:
			// index to CLIP
			{
				// note: if value is zero, not used
				int iIxSize = 0;
				int iIndex = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_TIMG:
			// like RIMG but for refraction:
			// index to CLIP
			{
				// note: if value is zero, not used
				int iIxSize = 0;
				int iIndex = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_RSAN:
			// reflection map image seam angle (in radians)
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_RBLR:
			// reflection-blur percentage
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_TBLR:
			// refraction-blur percentage
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_RIND:
			// refractive index
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_TROP:
			// transparency options
			{
				unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_CLRH:
			// color highlights
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_CLRF:
			// color filter
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_ADTR:
			// additive transparency
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_GLOW:
			// glow effect
			{
				int iIxSize = 0;

				unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);

				float fIntensity = BSwapF((*((float*)pBufPos)));
				pBufPos = (pBufPos +4);

				iIxSize = 0;
				// intensity-envelope
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);

				float fSize = BSwapF((*((float*)pBufPos)));
				pBufPos = (pBufPos +4);

				iIxSize = 0;
				// size-envelope
				int iSizeEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_GVAL:
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;

		case ID_LINE:
			// render outlines
			{
				/*
				int iIxSize = 0;

				unsigned short wFlags = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);

				float fSize = BSwapF((*((float*)pBufPos)));
				pBufPos = (pBufPos +4);

				iIxSize = 0;
				// size-envelope
				int iSizeEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
				*/
			}
			break;

		case ID_ALPH:
			// alpha-mode
			{
				unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);

				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);
			}
			break;

		case ID_VCOL:
			// vertex color map
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);

				unsigned int uiVmapType = MakeTag(pBufPos);
				pBufPos = (pBufPos +4);

				unsigned int uiStrSize = 0;
				string szName = GetPaddedString(pBufPos, uiStrSize);
				pBufPos = (pBufPos + uiStrSize);
			}
			break;
		}

		// skip to next sub-chunk (if any)
		// note: check if any of above changed value..
		if (pBufPos != pSubChunkEnd)
		{
			pBufPos = (pBufPos +usSCSize);
		}
	}

	pCurrentLayer->AddChunkToLayer(pSurfaces);
	m_ObjectData.AddChunk(pSurfaces);
	return true;
}

bool CLwoReader::Handle_LWOB_ID_SURF(const char *pChunk, const unsigned int uiChunkSize)
{
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	// in LWOB, there might not be layers
	// -> add a default layer for us
	if (pCurrentLayer == NULL)
	{
		pCurrentLayer = new CLwoLayer(0);
		m_ObjectData.AddChunk(pCurrentLayer);
	}

	// in LWOB, polygons have surface-index to which they use (1-based)
	CLwoSurface *pSurfaces = new CLwoSurface(pCurrentLayer->m_uiLayerIndex);

	char *pBufPos = (char*)pChunk;

	// count end of chunk for handling
	const char *pSurfEnd = (pBufPos + uiChunkSize);

	unsigned int uiStrSize = 0;

	// name of this surface
	pSurfaces->m_szSurfaceName = GetPaddedString(pBufPos, uiStrSize);
	pBufPos = (pBufPos + uiStrSize);

	while (pBufPos != pSurfEnd)
	{
		unsigned short usSCSize = 0;
		unsigned int uiSCType = GetSubChunkType(pBufPos, usSCSize);
		pBufPos = (pBufPos +6); // type (4) + size (2) bytes

		// count end of sub-chunk
		const char *pSubChunkEnd = (pBufPos + usSCSize);
		switch (uiSCType)
		{
		case ID_COLR:
			// three color-values (RGB) in 1-byte integers,
			// and one byte which is unused in LWOB and should be zero
			{
				// TODO: keep in object-datalist
				char *pcRGB = new char[3];

				for (int i = 0; i < 3; i++)
				{
					pcRGB[i] = pBufPos[i];
				}
				pBufPos = (pBufPos +3);

				// TODO: keep buffer instead of deleting
				delete pcRGB;

				// should be zero in older LWOB-format (not used)
				int iEnvelope = (char)(*pBufPos);
				pBufPos = (pBufPos +1);
			}
			break;

		case ID_FLAG:
			{
				unsigned short wFlags = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_LUMI:
		case ID_DIFF:
		case ID_SPEC:
		case ID_REFL:
		case ID_TRAN:
			// if any of these is missing, value of zero is assumed for it
			{
				unsigned short wSurProp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_GLOS:
			// needed only if specular-setting is non-zero above..
			{
				unsigned short wSurProp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_RIMG:
			// name of file for reflection map
			// or sequence of files
			{
				unsigned int uiStrSize = 0;
				string szFilename = GetPaddedString(pBufPos, uiStrSize);
				pBufPos = (pBufPos + uiStrSize);

				// if last part of string is " (sequence)",
				// then string defines _prefix_ for sequence of images
				// -> append three digit frame number for actual name
			}
			break;

		case ID_TIMG:
			// like RIMG, may have sequence of files
			{
				unsigned int uiStrSize = 0;
				string szFilename = GetPaddedString(pBufPos, uiStrSize);
				pBufPos = (pBufPos + uiStrSize);

				// if last part of string is " (sequence)",
				// then string defines _prefix_ for sequence of images
				// -> append three digit frame number for actual name
			}
			break;

		case ID_RSAN:
			// heading angle of 
			// reflection map seam
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);
			}
			break;

		case ID_RIND:
			// refractive index
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);
			}
			break;

		case ID_EDGE:
			// edge transparency
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);
			}
			break;

		case ID_SMAN:
			// max. smooth-shading angle between polygons (in degrees)
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);
			}
			break;

		case ID_CTEX:
		case ID_DTEX:
		case ID_STEX:
		case ID_RTEX:
		case ID_TTEX:
		case ID_BTEX:
			// if any of these, surface has texture for bump/color/diffuse..
			{
				// when detecting one of these,
				// all following in SURF-chunk refer to this texture
				// until one of these is detected again

				unsigned int uiStrSize = 0;
				string szTextureType = GetPaddedString(pBufPos, uiStrSize);
				pBufPos = (pBufPos + uiStrSize);
			}
			break;

		case ID_TFLG:
			// flags for current texture
			{
				unsigned short wFlags = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_TSIZ:
		case ID_TCTR:
		case ID_TFAL:
		case ID_TVEL:
			// XYZ-components of 
			// texture's size, center, falloff, velocity
			{
				float *pfTex = new float[3];
				float *pfChunkTex = (float*)pBufPos;
				for (int i = 0; i < 3; i++)
				{
					pfTex[i] = BSwapF(pfChunkTex[i]);
				}
				pBufPos = (pBufPos + 3*sizeof(float));

				// TODO: keep buffer instead of deleting
				delete pfTex;
			}
			break;

		case ID_TCLR:
			// texture color (should also have CTEX before this)
			{
				// TODO: keep in object-datalist
				char *pcRGB = new char[3];

				for (int i = 0; i < 3; i++)
				{
					pcRGB[i] = pBufPos[i];
				}
				pBufPos = (pBufPos +3);

				// TODO: keep buffer instead of deleting
				delete pcRGB;

				// should be zero in older LWOB-format (not used)
				int iEnvelope = (char)(*pBufPos);
				pBufPos = (pBufPos +1);
			}
			break;

		case ID_TVAL:
			// texture value of a diffuse/specular/reflection/transparency texture
			{
				unsigned short wTexProp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_TAMP:
			// amplitude of current bump-texture (should have BTEX before)
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);
			}
			break;

		case ID_TFRQ:
			// number of noise frequencies or wave sources for current texture
			{
				unsigned short wTexProp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

			/*
		case ID_TSP0:
		case ID_TSP1:
		case ID_TSP2:
			// texture parameters
			// (depend on order of buttons in GUI..)
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);
			}
			break;
			*/

			/*
		case ID_TFP0:
		case ID_TFP1:
		case ID_TFP2:
		case ID_TFP3:
			// same as above but on newer pre-6.0 LW only? (5.6?)
			break;
			*/
		}

		// skip to next sub-chunk (if any)
		// note: check if any of above changed value..
		if (pBufPos != pSubChunkEnd)
		{
			pBufPos = (pBufPos +usSCSize);
		}
	}

	pCurrentLayer->AddChunkToLayer(pSurfaces);
	m_ObjectData.AddChunk(pSurfaces);
	return true;
}

bool CLwoReader::Handle_LWO2_ID_ENVL(const char *pChunk, const unsigned int uiChunkSize)
{
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	char *pBufPos = (char*)pChunk;

	// count end of chunk for handling
	const char *pEnd = (pBufPos + uiChunkSize);

	int iIxSize = 0;
	int iIndex = (int)GetVarlenIX(pBufPos, iIxSize);
	pBufPos = (pBufPos +iIxSize);

	while (pBufPos != pEnd)
	{
		unsigned short usSCSize = 0;
		unsigned int uiSCType = GetSubChunkType(pBufPos, usSCSize);
		pBufPos = (pBufPos +6); // type (4) + size (2) bytes

		// count end of sub-chunk
		const char *pSubChunkEnd = (pBufPos + usSCSize);

		switch (uiSCType)
		{
		case ID_TYPE:
			{
				BYTE bUsrFormat = (*pBufPos);
				pBufPos = (pBufPos +1);
				BYTE bType = (*pBufPos);
				pBufPos = (pBufPos +1);
			}
			break;

		case ID_PRE:
			{
				unsigned short wType = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_POST:
			{
				unsigned short wType = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		case ID_KEY:
			{
				float fKey = BSwapF((*((float*)pBufPos)));
				pBufPos = (pBufPos +4);

				float fValue = BSwapF((*((float*)pBufPos)));
				pBufPos = (pBufPos +4);
			}
			break;

			/*
		case ID_SPAN:
			break;

		case ID_CHAN:
			break;
			*/

		case ID_NAME:
			{
				unsigned int uiStrSize = 0;
				string szName = GetPaddedString(pBufPos, uiStrSize);
				pBufPos = (pBufPos + uiStrSize);
			}
			break;
		}

		// skip to next sub-chunk (if any)
		// note: check if any of above changed value..
		if (pBufPos != pSubChunkEnd)
		{
			pBufPos = (pBufPos +usSCSize);
		}

	}

	return true;
}

bool CLwoReader::Handle_LWO2_ID_CLIP(const char *pChunk, const unsigned int uiChunkSize)
{
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	char *pBufPos = (char*)pChunk;

	// count end of chunk for handling
	const char *pEnd = (pBufPos + uiChunkSize);

	unsigned int uiIndex = BSwap4i((unsigned int*)pBufPos);
	pBufPos = (pBufPos +4);

	unsigned int uiClipType = MakeTag(pBufPos);
	pBufPos = (pBufPos +4);

	while (pBufPos != pEnd)
	{
		unsigned short usSCSize = 0;
		unsigned int uiSCType = GetSubChunkType(pBufPos, usSCSize);
		pBufPos = (pBufPos +6); // type (4) + size (2) bytes

		// count end of sub-chunk
		const char *pSubChunkEnd = (pBufPos + usSCSize);

		switch (uiSCType)
		{
		case ID_STIL:
			{
				unsigned int uiStrSize = 0;
				string szName = GetPaddedString(pBufPos, uiStrSize);
				pBufPos = (pBufPos + uiStrSize);
			}
			break;
		case ID_ISEQ:
			break;
		case ID_ANIM:
			break;
		case ID_XREF:
			break;
		case ID_STCC:
			break;
		case ID_TIME:
			{
				float fStart = BSwapF((*((float*)pBufPos)));
				pBufPos = (pBufPos +4);

				float fDuration = BSwapF((*((float*)pBufPos)));
				pBufPos = (pBufPos +4);

				float fFrameRate = BSwapF((*((float*)pBufPos)));
				pBufPos = (pBufPos +4);
			}
			break;
		case ID_CONT:
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos +iIxSize);
			}
			break;
		case ID_BRIT:
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos +iIxSize);
			}
			break;
		case ID_SATR:
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos +iIxSize);
			}
			break;
		case ID_HUE:
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos +iIxSize);
			}
			break;
		case ID_GAMM:
			{
				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos +iIxSize);
			}
			break;
		case ID_NEGA:
			{
				unsigned short wEnable = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;
		case ID_IFLT:
			break;
		case ID_PFLT:
			break;
		}

		// skip to next sub-chunk (if any)
		// note: check if any of above changed value..
		if (pBufPos != pSubChunkEnd)
		{
			pBufPos = (pBufPos +usSCSize);
		}
	}

	return true;
}

bool CLwoReader::Handle_LWO2_ID_VMAP(const char *pChunk, const unsigned int uiChunkSize)
{
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	// points-list this refers to
	CLwoPoints *pPrevPoints = (CLwoPoints*)m_ObjectData.GetPreviousOfType(ID_PNTS);

	char *pBufPos = (char*)pChunk;

	unsigned int uiVmapType = MakeTag(pBufPos);
	pBufPos = (pBufPos +4);

	/* is this needed?
	switch (uiVmapType)
	{
	case ID_PICK:
	case ID_WGHT:
	case ID_MNVW:
	case ID_TXUV:
	case ID_RGB:
	case ID_RGBA:
	case ID_MORF:
	case ID_SPOT:
		break;
	}
	*/

	unsigned short wDimension = BSwap2s((unsigned short*)pBufPos);
	pBufPos = (pBufPos +2);

	unsigned int uiStrSize = 0;
	string szName = GetPaddedString(pBufPos, uiStrSize);
	pBufPos = (pBufPos + uiStrSize);

	for (int i = 0; i < wDimension; i++)
	{
		int iIxSize = 0;
		int iVertIndex = (int)GetVarlenIX(pBufPos, iIxSize);
		pBufPos = (pBufPos +iIxSize);

		float *pfBuf = (float*)pBufPos;
		float fTemp = BSwapF(*pfBuf);
		pBufPos = (pBufPos +4);
	}

	return true;
}

bool CLwoReader::Handle_LWO2_ID_VMAD(const char *pChunk, const unsigned int uiChunkSize)
{
	CLwoLayer *pCurrentLayer = (CLwoLayer*)m_ObjectData.GetPreviousOfType(ID_LAYR);

	// points-list this refers to
	CLwoPoints *pPrevPoints = (CLwoPoints*)m_ObjectData.GetPreviousOfType(ID_PNTS);

	char *pBufPos = (char*)pChunk;

	unsigned int uiVmadType = MakeTag(pBufPos);
	pBufPos = (pBufPos +4);

	/* is this needed?
	switch (uiVmadType)
	{
	case ID_PICK:
	case ID_WGHT:
	case ID_MNVW:
	case ID_TXUV:
	case ID_RGB:
	case ID_RGBA:
	case ID_MORF:
	case ID_SPOT:
		break;
	}
	*/

	unsigned short wDimension = BSwap2s((unsigned short*)pBufPos);
	pBufPos = (pBufPos +2);

	unsigned int uiStrSize = 0;
	string szName = GetPaddedString(pBufPos, uiStrSize);
	pBufPos = (pBufPos + uiStrSize);

	for (int i = 0; i < wDimension; i++)
	{
		int iIxSize = 0;
		int iVertIndex = (int)GetVarlenIX(pBufPos, iIxSize);
		pBufPos = (pBufPos +iIxSize);

		int iPolIndex = (int)GetVarlenIX(pBufPos, iIxSize);
		pBufPos = (pBufPos +iIxSize);

		float *pfBuf = (float*)pBufPos;
		float fTemp = BSwapF(*pfBuf);
		pBufPos = (pBufPos +4);
	}

	return true;
}

// this is testing
char *CLwoReader::Handle_LWO2_BLOK(char *pBufPos, const unsigned short usBLOKSize)
{
	const char *pBlokChunkEnd = (pBufPos + usBLOKSize);

	// type of sub-chunk in this BLOK
	// (IMAP, TMAP, PROC, GRAD, SHDR)
	unsigned short usBlokCSize = 0;
	unsigned int uiBlokCType = GetSubChunkType(pBufPos, usBlokCSize);
	pBufPos = (pBufPos +6); // type (4) + size (2) bytes

	// keep separately..
	// read each blok one at a time..
	const char *pBlokHeaderEnd = (pBufPos + usBlokCSize);

	// ordinal string next
	unsigned int uiStrSize = 0;
	string szBlokOrdinal = GetPaddedString(pBufPos, uiStrSize);
	pBufPos = (pBufPos + uiStrSize);

	// handle header-attributes now
	while (pBufPos != pBlokHeaderEnd)
	{
		// now actual sub-sub-chunks of sub-chunk in BLOK?
		unsigned short usSBCSize = 0;
		unsigned int uiSBCType = GetSubChunkType(pBufPos, usSBCSize);
		pBufPos = (pBufPos +6); // type (4) + size (2) bytes

		// blok-header attributes (CHAN, ENAB, OPAC, AXIS)
		switch (uiSBCType)
		{
		case ID_CHAN:
			{
				// COLR, DIFF, LUMI, SPEC, GLOS, REFL, TRAN, RIND, TRNL, or BUMP:
				// see "normal" sub-chunks
				unsigned int uiChanType = MakeTag(pBufPos);
				pBufPos = (pBufPos +4);
			}
			break;
		case ID_ENAB:
			{
				unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;
		case ID_OPAC:
			{
				unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);

				float *pfBuf = (float*)pBufPos;
				float fTemp = BSwapF(*pfBuf);
				pBufPos = (pBufPos +4);

				// also index to envelope
				int iIxSize = 0;
				int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
				pBufPos = (pBufPos + iIxSize);
			}
			break;
		case ID_AXIS:
			{
				unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
				pBufPos = (pBufPos +2);
			}
			break;

		default:
			// skip past unknown type
			pBufPos = (pBufPos +usSBCSize);
			break;
		}
	}

	// now handle other sub-BLOK-chunks

	pBufPos = Handle_LWO2_BLOK_Chunks(pBufPos);

	// continue on upper-level chunk start
	return pBufPos;
}

// this is testing
char *CLwoReader::Handle_LWO2_BLOK_Chunks(char *pBufPos)
{
	// sub-bloks and their chunks
	// TMAP : CNTR, SIZE, ROTA, OREF, FALL, CSYS
	// IMAP : PROJ, AXIS, IMAG, WRAP, WRPW, WRPH, VMAP, AAST, PIXB, STCK, TAMP
	// PROC : AXIS, VALU, FUNC
	// GRAD : PNAM, INAM, GRST, GREN, GRPT, FKEY, IKEY
	// SHDR : FUNC

	// should get one of sub-bloks now
	unsigned short usBlokCSize = 0;
	unsigned int uiBlokCType = GetSubChunkType(pBufPos, usBlokCSize);
	pBufPos = (pBufPos +6); // type (4) + size (2) bytes

	// count after incrementing
	const char *pBlokEnd = (pBufPos + usBlokCSize);

	while (pBufPos != pBlokEnd)
	{
		// now actual sub-sub-chunks of sub-chunk in BLOK?
		unsigned short usSBCSize = 0;
		unsigned int uiSBCType = GetSubChunkType(pBufPos, usSBCSize);

		// did not handle padding correctly?
		if (uiSBCType == ID_BLOK)
		{
			return pBufPos;
		}

		pBufPos = (pBufPos +6); // type (4) + size (2) bytes

		// this is according to given sub-blok type
		if (uiBlokCType == ID_TMAP)
		{
			// texture-map attributes
			switch (uiSBCType)
			{
				/*
			case ID_CNTR:
				break;
			case ID_SIZE:
				break;
			case ID_ROTA:
				break;
			case ID_OREF:
				break;
				*/
			case ID_FALL:
				{
					unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
					pBufPos = (pBufPos +2);

					float *pfVals = new float[3];
					float *pfChunk = (float*)pBufPos;
					for (int i = 0; i < 3; i++)
					{
						pfVals[i] = BSwapF(pfChunk[i]);
					}
					pBufPos = (pBufPos + 3*sizeof(float));

					// TODO: keep buffer instead of deleting
					delete pfVals;

					// also index to envelope
					int iIxSize = 0;
					int iEnvelope = (int)GetVarlenIX(pBufPos, iIxSize);
					pBufPos = (pBufPos + iIxSize);
				}
				break;
			case ID_CSYS:
				{
					unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
					pBufPos = (pBufPos +2);
				}
				break;
			default:
				// skip past unknown type
				pBufPos = (pBufPos +usSBCSize);
				break;
			}
		}
		else if (uiBlokCType == ID_IMAP)
		{
			// image-map attributes
			switch (uiSBCType)
			{
			case ID_PROJ:
				{
					unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
					pBufPos = (pBufPos +2);
				}
				break;
			case ID_AXIS:
				{
					unsigned short wTemp = BSwap2s((unsigned short*)pBufPos);
					pBufPos = (pBufPos +2);
				}
				break;
				/*
			case ID_IMAG:
			case ID_WRAP:
			case ID_WRPW:
			case ID_WRPH:
			case ID_VMAP:
			case ID_AAST:
			case ID_PIXB:
			case ID_STCK:
			case ID_TAMP:
				break;
				*/
			default:
				// skip past unknown type
				pBufPos = (pBufPos +usSBCSize);
				break;
			}
		}
		else if (uiBlokCType == ID_PROC)
		{
			// procedural texture attributes
			switch (uiSBCType)
			{
				/*
			case ID_AXIS:
				break;
			case ID_VALU:
				break;
			case ID_FUNC:
				break;
				*/
			default:
				// skip past unknown type
				pBufPos = (pBufPos +usSBCSize);
				break;
			}
		}
		else if (uiBlokCType == ID_GRAD)
		{
			// gradient texture attributes
			switch (uiSBCType)
			{
				/*
			case ID_PNAM:
			case ID_INAM:
			case ID_GRST:
			case ID_GREN:
			case ID_GRPT:
			case ID_FKEY:
			case ID_IKEY:
				break;
				*/
			default:
				// skip past unknown type
				pBufPos = (pBufPos +usSBCSize);
				break;
			}
		}
		else if (uiBlokCType == ID_SHDR)
		{
			// shader-program attributes
			switch (uiSBCType)
			{
				/*
			case ID_FUNC:
				{
					// algorithm name and data (shader-code)
				}
				break;
				*/
			default:
				// skip past unknown type
				pBufPos = (pBufPos +usSBCSize);
				break;
			}
		}
		else
		{
			// abort, skip to end
			pBufPos = (char*)pBlokEnd;
			break;
		}
	}

	return pBufPos;
}


///////////// public methods

CLwoReader::CLwoReader()
: m_uiLWOSize(0)
, m_uiLwoFileType(0)
, m_ObjectData()
{
}

CLwoReader::~CLwoReader()
{
}

bool CLwoReader::ProcessFromFile(CMemFile &LwoFile)
{
	// handle file header first:
	// check we have valid IFF-header in there
	if (HandleFileHeader(LwoFile.GetAtOffset(0, 12), LwoFile.GetFilesize()) == false)
	{
		return false;
	}

	bool bRet = true;

	// start after file IFF-header to process chunks
	unsigned int uiChunkOffset = 12;
	while (uiChunkOffset < m_uiLWOSize
		&& bRet == true)
	{
		// we need at least type+size for next chunk (8 bytes):
		// get type and size from the chunk header
		//
		const char *pChunk = LwoFile.GetAtOffset(uiChunkOffset, 8);
		unsigned int uiChunkSize = 0;
		unsigned int uiChunkType = GetChunkType(pChunk, uiChunkSize);

		uiChunkOffset += 8; // skip to actual data (past chunk type and size)

		// if next chunk is larger than remains in file -> error, abort
		if (uiChunkSize > (LwoFile.GetFilesize() - uiChunkOffset))
		{
			return false;
		}

		// verify we have the data in buffer 
		// for the actual chunk-data
		const char *pChunkData = LwoFile.GetAtOffset(uiChunkOffset, uiChunkSize);

		// handle each chunk in file
		bRet = ProcessChunk(
					pChunkData, 
					uiChunkType, 
					uiChunkSize);

		// determine offset of next chunk in file
		uiChunkOffset += uiChunkSize;
	}

	return bRet;
}

