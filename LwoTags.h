////////////////////////////////////
// LwoTags.h : definitions of Lightwave-object 
// tags for loading and parsing files
//
// Based on lwo2.h in Lightwave SDK example code
// by Ernie Wright  17 Sep 00
//
// Ilkka Prusi 2008
//


#ifndef _LWOTAGS_H_
#define _LWOTAGS_H_

/* chunk and subchunk IDs */

#define LWID_(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

/* Universal IFF identifiers */
/** IFF start-identifier **/
#define ID_FORM		LWID_('F','O','R','M')

/** 
 LW6.0 and later format: LWO2
**/
#define ID_LWO2		LWID_('L','W','O','2')

/** 
 pre-LW6.0 format: LWOB
 usually does not have layers and some other differences
**/
#define ID_LWOB		LWID_('L','W','O','B')

/** 
 "Layered object" format similar to LWOB/LWO2
  but only used in pre-LW6.0 Modeler?
**/
#define ID_LWLO		LWID_('L','W','L','O')


/**  PRIMARY CHUNK ID  **/
#define ID_LAYR		LWID_('L','A','Y','R')
#define ID_PNTS		LWID_('P','N','T','S')
#define ID_VMAP		LWID_('V','M','A','P')
#define ID_POLS		LWID_('P','O','L','S')
#define ID_TAGS		LWID_('T','A','G','S')
#define ID_PTAG		LWID_('P','T','A','G')
#define ID_ENVL		LWID_('E','N','V','L')
#define ID_CLIP		LWID_('C','L','I','P')
#define ID_SURF		LWID_('S','U','R','F')
#define ID_BBOX		LWID_('B','B','O','X')
#define ID_DESC		LWID_('D','E','S','C')
#define ID_TEXT		LWID_('T','E','X','T')
#define ID_ICON		LWID_('I','C','O','N')

/** pre-LW6.0 surface? (LWOB-only?) **/
#define ID_SRFS		LWID_('S','R','F','S')

/** pre-LW6.0 spline-curve list? (LWOB-only?) **/
#define ID_CRVS		LWID_('C','R','V','S')

/** LW6.5: Discontinuous Vertex Mapping **/
#define ID_VMAD		LWID_('V','M','A','D')

/** vertex map parameter **/
#define ID_VMPA		LWID_('V','M','P','A')

/** sub-chunks used in multiple primary-chunks **/
#define ID_NAME		LWID_('N','A','M','E')

/**  POLS TYPE  **/
#define ID_FACE		LWID_('F','A','C','E')
#define ID_CURV		LWID_('C','U','R','V')
#define ID_PTCH		LWID_('P','T','C','H')
#define ID_MBAL		LWID_('M','B','A','L')
#define ID_BONE		LWID_('B','O','N','E')

/**  PTAG TYPE  **/
#define ID_SURF		LWID_('S','U','R','F')
#define ID_PART		LWID_('P','A','R','T')
#define ID_SGMP		LWID_('S','G','M','P')
#define ID_BNID		LWID_('B','N','I','D')

/**  CLIP SUB-CHUNK ID  **/
#define ID_STIL		LWID_('S','T','I','L')
#define ID_ISEQ		LWID_('I','S','E','Q')
#define ID_ANIM		LWID_('A','N','I','M')
#define ID_XREF		LWID_('X','R','E','F')
#define ID_STCC		LWID_('S','T','C','C')
#define ID_TIME		LWID_('T','I','M','E')
#define ID_CONT		LWID_('C','O','N','T')
#define ID_BRIT		LWID_('B','R','I','T')
#define ID_SATR		LWID_('S','A','T','R')
#define ID_HUE		LWID_('H','U','E',' ')
#define ID_GAMM		LWID_('G','A','M','M')
#define ID_NEGA		LWID_('N','E','G','A')
#define ID_CROP		LWID_('C','R','O','P')
#define ID_ALPH		LWID_('A','L','P','H')
#define ID_COMP		LWID_('C','O','M','P')
#define ID_IFLT		LWID_('I','F','L','T')
#define ID_PFLT		LWID_('P','F','L','T')

/**  ENVELOPE SUB-CHUNK ID **/
#define ID_PRE		LWID_('P','R','E',' ')
#define ID_POST		LWID_('P','O','S','T')
#define ID_KEY		LWID_('K','E','Y',' ')
#define ID_SPAN		LWID_('S','P','A','N')
#define ID_CHAN		LWID_('C','H','A','N')
#define ID_NAME		LWID_('N','A','M','E')

/**  SURFACE SUB-CHUNK ID  **/
#define ID_COLR		LWID_('C','O','L','R')
#define ID_DIFF		LWID_('D','I','F','F')
#define ID_LUMI		LWID_('L','U','M','I')
#define ID_SPEC		LWID_('S','P','E','C')
#define ID_REFL		LWID_('R','E','F','L')
#define ID_TRAN		LWID_('T','R','A','N')
#define ID_TRNL		LWID_('T','R','N','L')
#define ID_GLOS		LWID_('G','L','O','S')
#define ID_SHRP		LWID_('S','H','R','P')
#define ID_BUMP		LWID_('B','U','M','P')
#define ID_SIDE		LWID_('S','I','D','E')
#define ID_SMAN		LWID_('S','M','A','N')
#define ID_CTEX		LWID_('C','T','E','X')
#define ID_DTEX		LWID_('D','T','E','X')
#define ID_STEX		LWID_('S','T','E','X')
#define ID_RTEX		LWID_('R','T','E','X')
#define ID_TTEX		LWID_('T','T','E','X')
#define ID_BTEX		LWID_('B','T','E','X')
#define ID_RFOP		LWID_('R','F','O','P')
#define ID_RIMG		LWID_('R','I','M','G')
#define ID_RSAN		LWID_('R','S','A','N')
#define ID_RBLR		LWID_('R','B','L','R')
#define ID_RIND		LWID_('R','I','N','D')
#define ID_EDGE		LWID_('E','D','G','E')
#define ID_TROP		LWID_('T','R','O','P')
#define ID_TIMG		LWID_('T','I','M','G')
#define ID_TBLR		LWID_('T','B','L','R')
#define ID_TFLG		LWID_('T','F','L','G')
#define ID_TSIZ		LWID_('T','S','I','Z')
#define ID_TCTR		LWID_('T','C','T','R')
#define ID_TFAL		LWID_('T','F','A','L')
#define ID_TVEL		LWID_('T','V','E','L')
#define ID_TCLR		LWID_('T','C','L','R')
#define ID_TVAL		LWID_('T','V','A','L')
#define ID_TFRQ		LWID_('T','F','R','Q')
#define ID_TAMP		LWID_('T','A','M','P')
//#define ID_TSP0		LWID_('T','S','P','0')
//#define ID_TSP1		LWID_('T','S','P','1')
//#define ID_TSP2		LWID_('T','S','P','2')
//#define ID_TFP0		LWID_('T','F','P','0')
//#define ID_TFP1		LWID_('T','F','P','1')
//#define ID_TFP2		LWID_('T','F','P','2')
//#define ID_TFP3		LWID_('T','F','P','3')
#define ID_CLRH		LWID_('C','L','R','H')
#define ID_CLRF		LWID_('C','L','R','F')
#define ID_ADTR		LWID_('A','D','T','R')
#define ID_GLOW		LWID_('G','L','O','W')
#define ID_LINE		LWID_('L','I','N','E')
#define ID_ALPH		LWID_('A','L','P','H')
#define ID_AVAL		LWID_('A','V','A','L')
#define ID_GVAL		LWID_('G','V','A','L')
#define ID_BLOK		LWID_('B','L','O','K')
#define ID_LCOL		LWID_('L','C','O','L')
#define ID_LSIZ		LWID_('L','S','I','Z')
#define ID_CMNT		LWID_('C','M','N','T')
#define ID_VCOL		LWID_('V','C','O','L')

/**  TEXTURE LAYER  **/
#define ID_CHAN		LWID_('C','H','A','N')
#define ID_TYPE		LWID_('T','Y','P','E')
#define ID_NAME		LWID_('N','A','M','E')
#define ID_ENAB		LWID_('E','N','A','B')
#define ID_OPAC		LWID_('O','P','A','C')
#define ID_FLAG		LWID_('F','L','A','G')
#define ID_PROJ		LWID_('P','R','O','J')
#define ID_STCK		LWID_('S','T','C','K')
#define ID_TAMP		LWID_('T','A','M','P')

/**  TEXTURE MAPPING  **/
#define ID_TMAP		LWID_('T','M','A','P')
#define ID_AXIS		LWID_('A','X','I','S')
#define ID_CNTR		LWID_('C','N','T','R')
#define ID_SIZE		LWID_('S','I','Z','E')
#define ID_ROTA		LWID_('R','O','T','A')
#define ID_OREF		LWID_('O','R','E','F')
#define ID_FALL		LWID_('F','A','L','L')
#define ID_CSYS		LWID_('C','S','Y','S')

/**  IMAGE MAP  **/
#define ID_IMAP		LWID_('I','M','A','P')
#define ID_IMAG		LWID_('I','M','A','G')
#define ID_WRAP		LWID_('W','R','A','P')
#define ID_WRPW		LWID_('W','R','P','W')
#define ID_WRPH		LWID_('W','R','P','H')
#define ID_VMAP		LWID_('V','M','A','P')
#define ID_AAST		LWID_('A','A','S','T')
#define ID_PIXB		LWID_('P','I','X','B')

/** Vertex mapping **/
/*
#define ID_PICK		LWID_('P','I','C','K')
#define ID_WGHT		LWID_('W','G','H','T')
#define ID_MNVW		LWID_('M','N','V','W')
#define ID_TXUV		LWID_('T','X','U','V')
#define ID_RGB		LWID_('R','G','B',' ')
#define ID_RGBA		LWID_('R','G','B','A')
#define ID_MORF		LWID_('M','O','R','F')
#define ID_SPOT		LWID_('S','P','O','T')
*/

/**  PROCEDURAL TEXTURE  **/
#define ID_PROC		LWID_('P','R','O','C')
#define ID_COLR		LWID_('C','O','L','R')
#define ID_VALU		LWID_('V','A','L','U')
#define ID_FUNC		LWID_('F','U','N','C')
#define ID_FTPS		LWID_('F','T','P','S')
#define ID_ITPS		LWID_('I','T','P','S')
#define ID_ETPS		LWID_('E','T','P','S')

/**  GRADIENT **/
#define ID_GRAD		LWID_('G','R','A','D')
#define ID_GRST		LWID_('G','R','S','T')
#define ID_GREN		LWID_('G','R','E','N')

/**  SHADER PLUGIN  */
#define ID_SHDR		LWID_('S','H','D','R')
#define ID_DATA		LWID_('D','A','T','A')


// handle generating constant tag types,
// make static instance of the class ?
//
/*
class CLwoTags
{
public:

	typedef enum tLWOTags
	{
		LTAG_UNKNOWN = 0,

		// IFF-file header tag
		LTAG_ID_FORM,
	};

public:
	CLwoTags(void) 
	{
		// construct tag definitions

		//  SHADER PLUGIN  
		//TAG_SHDR = MakeTag("SHDR");
		//TAG_DATA = MakeTag("DATA");
	};

	inline unsigned int MakeTag(const char *buf)
	{
		return (
			(unsigned long) (buf[0])<<24 
			| (unsigned long) (buf[1])<<16 
			| (unsigned long) (buf[2])<<8 
			| (unsigned long) (buf[3]));
	}
};
*/


#endif

