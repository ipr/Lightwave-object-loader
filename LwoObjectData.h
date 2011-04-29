//////////////////////////////////////////////////////////////////////
// LwoObjectData.h : container of parsed object-information
//
// Ilkka Prusi 2010
//

#pragma once

#ifndef _LWOOBJECTDATA_H_
#define _LWOOBJECTDATA_H_

#include "LwoTags.h"

#include <map>
#include <string>
#include <vector>
using namespace std;

// base-class for all other chunk-type data
class CLwoChunk
{
public:
	// ID of type of this chunk
	unsigned int m_uiChunkType;

	// zero-base index of the layer (parent or inherited object when layer)
	unsigned int m_uiLayerIndex;

	// TODO: pointer to parent-chunks?
	// (such as poly-to-layer?)
	//CLwoChunk *m_pParentChunk;

public:
	CLwoChunk(const unsigned int uiChunkType, const unsigned int uiLayerIndex)
		: m_uiChunkType(uiChunkType)
		, m_uiLayerIndex(uiLayerIndex)
	{};
	virtual ~CLwoChunk()
	{};
};

// tags: list of tag names
class CLwoTagnameList : public CLwoChunk
{
public:
	// other places may have zero-based index to tag-name:
	// keep that data here
	typedef map<long, string> tTagList;
	tTagList m_TagnameList;

public:
	// no index of layer, this is upper-level
	CLwoTagnameList(void)
		: CLwoChunk(ID_TAGS, 0)
	{};
	virtual ~CLwoTagnameList()
	{
		m_TagnameList.clear();
	};

	bool AddTagname(string &szTagname)
	{
		long lNewIndex = (long)m_TagnameList.size();
		m_TagnameList.insert(tTagList::value_type(lNewIndex, szTagname));
		return true;
	};

	string GetTagname(long lIndex)
	{
		tTagList::iterator itTags = m_TagnameList.find(lIndex);
		if (itTags != m_TagnameList.end())
		{
			return itTags->second;
		}
		// not found
		return string();
	};
};


// use list for chunks of data
typedef vector<CLwoChunk*> tChunkList;

// layer: collection of points/polygons/vectors/surfaces etc.
// can have pivot point and other info directly on it also
class CLwoLayer : public CLwoChunk
{
public:
	// if parent-layer for layer is given, keep here:
	// zero-based value,
	// can be -1 if not specified
	int m_iParentLayerIndex;

	// TODO: replace index by pointer to parent-layer? (if any?)
	//CLwoLayer *m_pParentLayer;

	// triplet of floats (vertex, XYZ)
	// for layer pivot-point
	float *m_pfPivotPoint;

	unsigned short m_usLayerNumber;
	unsigned short m_usLayerFlags;

	// name of this layer (if given)
	string m_szLayerName;

	// list of pointers for easier access,
	// don't destroy here since CLwoObjectData 
	// has "true" list of data (with release) 
	tChunkList m_ChunksInLayer;

public:
	// zero-base index of the layer (this)
	CLwoLayer(const unsigned int uiLayerIndex)
		: CLwoChunk(ID_LAYR, uiLayerIndex)
		, m_iParentLayerIndex(-1)
		, m_pfPivotPoint(NULL)
		, m_usLayerNumber(0)
		, m_usLayerFlags(0)
		, m_szLayerName()
		, m_ChunksInLayer()
	{};
	virtual ~CLwoLayer()
	{
		if (m_pfPivotPoint != NULL)
		{
			delete m_pfPivotPoint;
			m_pfPivotPoint = NULL;
		}

		// don't destroy objects here,
		// only remove the pointers (see CLwoObjectData)
		m_ChunksInLayer.clear();
	};

	// reference-list only so that we can easily
	// locate data in a single layer
	bool AddChunkToLayer(CLwoChunk *pChunk)
	{
		m_ChunksInLayer.push_back(pChunk);
		return true;
	};
};

// bounding box: extents of the layer
class CLwoBoundingBox : public CLwoChunk
{
public:
	// array of floats with the extents of the layer
	//
	float *m_pfBoxExtents;

	// amount of values in buffer (size of buffer)
	long m_lValueCount;

public:
	// zero-base index of the layer (parent)
	CLwoBoundingBox(const unsigned int uiLayerIndex)
		: CLwoChunk(ID_BBOX, uiLayerIndex)
		, m_lValueCount(0)
		, m_pfBoxExtents(NULL)
	{};
	virtual ~CLwoBoundingBox()
	{
		if (m_pfBoxExtents != NULL)
		{
			delete m_pfBoxExtents;
			m_pfBoxExtents = NULL;
		}
		m_lValueCount = 0;
	};
};


// combine vertex-coordinates and polygon-indices
// to this single object?
//class CLwoPolyPoints : public CLwoChunk

// points: list of points in world-coordinates
class CLwoPoints : public CLwoChunk
{
public:
	// array of floats with coordinates of points
	// which may be shared by vertices (before adding normals)
	//
	float *m_pfPointList;

	// amount of values in buffer (size of buffer)
	long m_lValueCount;

	// TODO: keep reference to polygon-data here?

public:
	CLwoPoints(const unsigned int uiLayerIndex)
		: CLwoChunk(ID_PNTS, uiLayerIndex)
		, m_lValueCount(0)
		, m_pfPointList(NULL)
	{};
	virtual ~CLwoPoints()
	{
		if (m_pfPointList != NULL)
		{
			delete m_pfPointList;
			m_pfPointList = NULL;
		}
		m_lValueCount = 0;
	};
};

// polygons: index-list referring to points
// to describe where edges of polygon are
class CLwoPolygons : public CLwoChunk
{
public:
	// a "row" in poly definition
	// (x indices as part of sub-chunk)
	class CLwoPolyRow
	{
	public:
		long m_lPolyRowIndex;
		unsigned short m_wVertexCount;
		unsigned short m_wFlags;

		// TODO:
		// in older LWOB-format, we can have
		// 1-based surface-index in each row
		unsigned short m_wSurfaceIndex;
		// use pointer to surface to simplify later?
		//CLwoChunk *m_pSurface;

		// TODO: also, each poly may have detail-polygons

		// list of indices (keep as int for simplicity, 
		// varying sizes in source data),
		// see m_wVertexCount for count (length of array)
		// 
		int *m_piIndices;

	public:
		CLwoPolyRow(const long lPolyRowIndex, const unsigned short wVertexCount, const unsigned short wFlags)
			: m_lPolyRowIndex(lPolyRowIndex)
			, m_wVertexCount(wVertexCount)
			, m_wFlags(wFlags)
			, m_wSurfaceIndex(0)
			, m_piIndices(NULL)
		{
			m_piIndices = new int[m_wVertexCount];
		};
		~CLwoPolyRow(void)
		{
			if (m_piIndices != NULL)
			{
				delete m_piIndices;
				m_piIndices = NULL;
			}
		};
	};

public:
	// sub-chunk type:
	// FACE, CURV, PTCH, MBAL or BONE
	unsigned int m_uiPolyTypeID;

	// keep arrays of indices
	// (may have different sizes)
	// which refer to point-list
	typedef vector<CLwoPolyRow*> tPolyList;
	tPolyList m_PolyList;

	// keep reference to points
	CLwoPoints *m_pPointsList;

public:
	CLwoPolygons(const unsigned int uiLayerIndex)
		: CLwoChunk(ID_POLS, uiLayerIndex)
		, m_uiPolyTypeID(0)
		, m_pPointsList(NULL)
	{};
	virtual ~CLwoPolygons()
	{
		tPolyList::iterator itList = m_PolyList.begin();
		tPolyList::iterator itListEnd = m_PolyList.end();
		while (itList != itListEnd)
		{
			CLwoPolyRow *pRow = (*itList);
			if (pRow != NULL)
			{
				delete pRow;
			}
			++itList;
		}
		m_PolyList.clear();

		// don't delete, only reference here
		m_pPointsList = NULL;
	};
};


// polygon-related tags,
// such as surface-polygon mapping
class CLwoPolyTags : public CLwoChunk
{
public:
	// sub-chunk type (mapping type):
	// SURF, PART, SMGP
	unsigned int m_uiPtagTypeID;

	// pair: polygon index, tag index
	typedef pair<int, int> tPolToTag;
	typedef vector<tPolToTag> tPolyTagList;
	tPolyTagList m_PolyTagList;

	// reference to polygon-list this is related to
	CLwoPolygons *m_pPolyList;

public:
	CLwoPolyTags(const unsigned int uiLayerIndex)
		: CLwoChunk(ID_PTAG, uiLayerIndex)
		, m_uiPtagTypeID(0)
		, m_pPolyList(NULL)
	{};
	virtual ~CLwoPolyTags()
	{
		m_PolyTagList.clear();

		// don't delete here, just reference
		m_pPolyList = NULL;
	};
};


// surface: color-list or image (texture)
// for polygon
class CLwoSurface : public CLwoChunk
{
	/*
public:
	// sub-chunk of surface
	class CSurfacePart
	{
	public:
		unsigned int m_uiSurfTypeID;

	};

	// or:
	class CTexture
	{
	};

	*/

public:
	// name of this surface
	string m_szSurfaceName;

	// name of parent surface (if any)
	string m_szParentSurfaceName;

public:
	CLwoSurface(const unsigned int uiLayerIndex)
		: CLwoChunk(ID_SURF, uiLayerIndex)
		, m_szSurfaceName()
		, m_szParentSurfaceName()
	{};
	virtual ~CLwoSurface()
	{};
};

class CLwoEnvelope : public CLwoChunk
{
	/*
public:
	// sub-chunk of envelope
	class CEnvelopePart
	{
	public:
		unsigned int m_uiEnvlTypeID;

	};
	*/

public:
	int m_iEnvelopeIndex;

public:
	CLwoEnvelope(const unsigned int uiLayerIndex)
		: CLwoChunk(ID_ENVL, uiLayerIndex)
		, m_iEnvelopeIndex(0)
	{};
	virtual ~CLwoEnvelope()
	{};
};

class CLwoClip : public CLwoChunk
{
public:
	CLwoClip(const unsigned int uiLayerIndex)
		: CLwoChunk(ID_CLIP, uiLayerIndex)
	{};
	virtual ~CLwoClip()
	{};
};

// TODO: also for VMAD (discontinuous mapping)?
class CLwoVertexMap : public CLwoChunk
{
public:
	CLwoVertexMap(const unsigned int uiLayerIndex)
		: CLwoChunk(ID_VMAP, uiLayerIndex)
	{};
	virtual ~CLwoVertexMap()
	{};
};

//////////////////
// container for all layers and other chunks
// in the LWO-object data.
//
// contains "actual" chunk-objects such that
// this will release all created chunk-objects,
// chunk-objects may have reference-pointers to each other.
//
class CLwoObjectData
{
protected:
	// list of all chunks found for the object in file:
	// when destroying this list should also destroy objects
	// to release memory (see destructor here)
	tChunkList m_ChunkList;

	// zero-based index for next layer (if any),
	// counter when adding layers for simplicity
	unsigned int m_uiNextLayerIndex;

	inline CLwoChunk *GetNextOfType(const unsigned int uiType, tChunkList &ChunkList, tChunkList::iterator &itCurPos);

public:
	CLwoObjectData(void)
		: m_uiNextLayerIndex(0) // zero-based
	{};
	~CLwoObjectData(void)
	{
		tChunkList::iterator itChunks = m_ChunkList.begin();
		tChunkList::iterator itChunksEnd = m_ChunkList.end();
		while (itChunks != itChunksEnd)
		{
			CLwoChunk *pChunk = (*itChunks);
			if (pChunk != NULL)
			{
				delete pChunk;
			}
			++itChunks;
		}
		m_ChunkList.clear();
	};

	bool AddChunk(CLwoChunk *pChunk)
	{
		// this list is used when eventually 
		// releasing object from memory,
		// other objects may link to each other 
		// but they should not destroy others
		m_ChunkList.push_back(pChunk);
		return true;
	};

	CLwoChunk *GetPreviousOfType(const unsigned int uiType)
	{
		// loop list if reverse to locate previously added of given type
		tChunkList::reverse_iterator itChunks = m_ChunkList.rbegin();
		tChunkList::reverse_iterator itChunksEnd = m_ChunkList.rend();
		while (itChunks != itChunksEnd)
		{
			CLwoChunk *pChunk = (*itChunks);
			if (pChunk != NULL)
			{
				if (pChunk->m_uiChunkType == uiType)
				{
					return pChunk;
				}
			}
			++itChunks;
		}
		return NULL;
	};

	unsigned int GetNextLayerIndex()
	{
		unsigned int uiIndex = m_uiNextLayerIndex;
		m_uiNextLayerIndex++;
		return uiIndex;
	};

	// create internal links between 
	// related chunks and sub-chunks in the object data
	bool CreateObjectLinkage();

	//friend class CLwoReader;
};

// TODO: this is testing,
// generate vertices for polygons
// from point-coordinates and poly-indices
/*
class CVertexTriplet
{
public:
	// C++0x (test another compiler)
	//float m_fX{0};
	//float m_fY{0};
	//float m_fZ{0};

	float m_fX;
	float m_fY;
	float m_fZ;

};
*/

#endif // ifndef _LWOOBJECTDATA_H_

