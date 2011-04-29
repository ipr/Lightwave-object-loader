//////////////////////////////////////////////////////////////////////
// LwoObjectData.cpp : container of parsed object-information
//
// Ilkka Prusi 2010
//

#include "LwoObjectData.h"

CLwoChunk *CLwoObjectData::GetNextOfType(const unsigned int uiType, tChunkList &ChunkList, tChunkList::iterator &itCurPos)
{
	tChunkList::iterator itEnd = ChunkList.end();
	while (itCurPos != itEnd)
	{
		CLwoChunk *pChunk = (*itCurPos);
		if (pChunk != NULL)
		{
			if (pChunk->m_uiChunkType == uiType)
			{
				return pChunk;
			}
		}
		++itCurPos;
	}
	return NULL;
}

// create internal links between 
// related chunks and sub-chunks in the object data:
// when file has been parsed this is called
// to prepare information for actual using.
bool CLwoObjectData::CreateObjectLinkage()
{
	//vector<CVertexTriplet> vTriplets;

	// count polygons in poly-chunks 
	// to determine amount of triplets needed
	// (first locate each poly-chunk and get count)
	unsigned long ulPolyCount = 0;

	/*
	CVertexTriplet Tmp;
	Tmp.m_fX = 1;
	Tmp.m_fY = 2;
	Tmp.m_fZ = 3;
	vTriplets.push_back(Tmp);
	*/

	tChunkList::iterator itChunks = m_ChunkList.begin();
	tChunkList::iterator itChunksEnd = m_ChunkList.end();

	CLwoLayer *pLayer = (CLwoLayer*)GetNextOfType(ID_LAYR, m_ChunkList, itChunks);
	if (pLayer != NULL)
	{
		tChunkList::iterator itLChunks = pLayer->m_ChunksInLayer.begin();
		//tChunkList::iterator itLChunksEnd = pLayer->m_ChunksInLayer.end();

		CLwoPoints *pPoints = (CLwoPoints*)GetNextOfType(ID_PNTS, pLayer->m_ChunksInLayer, itLChunks);
	}

	/*
	while (itChunks != itChunksEnd)
	{
		CLwoChunk *pChunk = (*itChunks);
		if (pChunk != NULL)
		{
			if (pChunk->m_uiChunkType == ID_LAYR)
			{
				// find points-chunk,
				// then find each poly-chunk in layer
				// and generate polygon vertices
				// by point-coordinates and poly-indices
			}
		}
		++itChunks;
	}
	*/


	return false;
}
