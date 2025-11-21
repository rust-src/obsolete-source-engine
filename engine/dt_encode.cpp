//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "quakedef.h"
#include "dt.h"
#include "dt_encode.h"
#include "coordsize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern void DataTable_Warning( PRINTF_FORMAT_STRING const char *pInMessage, ... );
extern bool ShouldWatchThisProp( const SendTable *pTable, int objectID, const char *pPropName );

// The engine implements this.
extern const char* GetObjectClassName( int objectID );

void EncodeFloat( const SendPropPrecalc *pProp, float fVal, bf_write *pOut, int objectID )
{
	
}


static float DecodeFloat(SendPropPrecalc const *pProp, bf_read *pIn)
{
	
}

static inline void DecodeVector(SendPropPrecalc const *pProp, bf_read *pIn, float *v)
{
	
}

int	DecodeBits( DecodeInfo *pInfo, unsigned char *pOut )
{
	bf_read temp;

	// Read the property in (note: we don't return the bits from here because Decode returns
	// the decoded bits.. we're interested in getting the encoded bits).
	temp = *pInfo->m_pIn;
	pInfo->m_pRecvProp = NULL;
	pInfo->m_pData = NULL;
	g_PropTypeFns[pInfo->m_pProp->m_Type].Decode( pInfo );

	// Return the encoded bits.
	int nBits = pInfo->m_pIn->GetNumBitsRead() - temp.GetNumBitsRead();
	temp.ReadBits(pOut, nBits);
	return nBits;
}


// ---------------------------------------------------------------------------------------- //
// Most of the prop types can use this generic FastCopy version. Arrays are a bit of a pain.
// ---------------------------------------------------------------------------------------- //

inline void Generic_FastCopy( 
	const SendPropPrecalc *pSendPropPrecalc, 
	const RecvProp *pRecvProp, 
	const unsigned char *pSendData, 
	unsigned char *pRecvData,
	int objectID )
{
	// Get the data out of the ent.
	CRecvProxyData recvProxyData;

	pSendPropPrecalc->GetProxyFn()( 
		pSendPropPrecalc,
		pSendData, 
		pSendData + pSendPropPrecalc->GetOffset(),
		&recvProxyData.m_Value,
		0,
		objectID
		);

	// Fill in the data for the recv proxy.
	recvProxyData.m_pRecvProp = pRecvProp;
	recvProxyData.m_iElement = 0;
	recvProxyData.m_ObjectID = objectID;
	pRecvProp->GetProxyFn()( &recvProxyData, pRecvData, pRecvData + pRecvProp->GetOffset() );
}


// ---------------------------------------------------------------------------------------- //
// DecodeInfo implementation.
// ---------------------------------------------------------------------------------------- //

void DecodeInfo::CopyVars( const DecodeInfo *pOther )
{
	m_pStruct = pOther->m_pStruct;
	m_pData = pOther->m_pData;
	
	m_pRecvProp = pOther->m_pRecvProp;
	m_pProp = pOther->m_pProp;
	m_pIn = pOther->m_pIn;
	m_ObjectID = pOther->m_ObjectID;
	m_iElement = pOther->m_iElement;
}


// ---------------------------------------------------------------------------------------- //
// Int property type abstraction.
// ---------------------------------------------------------------------------------------- //

void Int_Encode( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp, bf_write *pOut, int objectID )
{
	
}


void Int_Decode( DecodeInfo *pInfo )
{


	if ( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}


int Int_CompareDeltas( const SendPropPrecalc *pProp, bf_read *p1, bf_read *p2 )
{
	return p1->CompareBits(p2, pProp->m_nBits);
}

const char* Int_GetTypeNameString()
{
	return "DPT_Int";
}


bool Int_IsZero( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp )
{
	return (pVar->m_Int == 0);
}


void Int_DecodeZero( DecodeInfo *pInfo )
{
	pInfo->m_Value.m_Int = 0;

	if ( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}

bool Int_IsEncodedZero( const SendPropPrecalc *pProp, bf_read *pIn )
{
	return pIn->ReadUBitLong( pProp->m_nBits ) == 0;
}

void Int_SkipProp( const SendPropPrecalc *pProp, bf_read *pIn )
{
	pIn->SeekRelative( pProp->m_nBits );
}

// ---------------------------------------------------------------------------------------- //
// Float type abstraction.
// ---------------------------------------------------------------------------------------- //

void Float_Encode( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp, bf_write *pOut, int objectID )
{
	EncodeFloat( pProp, pVar->m_Float, pOut, objectID );
}

void Float_Decode( DecodeInfo *pInfo )
{
	pInfo->m_Value.m_Float = DecodeFloat(pInfo->m_pProp, pInfo->m_pIn);

	if ( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}


int	Float_CompareDeltas( const SendPropPrecalc *pProp, bf_read *p1, bf_read *p2 )
{
	
}

const char* Float_GetTypeNameString()
{
	return "DPT_Float";
}


bool Float_IsZero( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp )
{
	return (pVar->m_Float == 0);
}


void Float_DecodeZero( DecodeInfo *pInfo )
{
	pInfo->m_Value.m_Float = 0;

	if ( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}

bool Float_IsEncodedZero( const SendPropPrecalc *pProp, bf_read *pIn )
{
	return DecodeFloat( pProp, pIn ) == 0.0f;
}

void Float_SkipProp( const SendPropPrecalc *pProp, bf_read *pIn )
{
	pIn->SeekRelative( sizeof(float) * 8 );
}


// ---------------------------------------------------------------------------------------- //
// Vector type abstraction.
// ---------------------------------------------------------------------------------------- //

void Vector_Encode( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp, bf_write *pOut, int objectID )
{
	EncodeFloat(pProp, pVar->m_Vector[0], pOut, objectID);
	EncodeFloat(pProp, pVar->m_Vector[1], pOut, objectID);
	EncodeFloat(pProp, pVar->m_Vector[2], pOut, objectID);
}


void Vector_Decode(DecodeInfo *pInfo)
{
	DecodeVector( pInfo->m_pProp, pInfo->m_pIn, pInfo->m_Value.m_Vector );
	
	if( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}


int	Vector_CompareDeltas( const SendPropPrecalc *pProp, bf_read *p1, bf_read *p2 )
{
	int c1 = Float_CompareDeltas( pProp, p1, p2 );
	int c2 = Float_CompareDeltas( pProp, p1, p2 );
	int c3 = Float_CompareDeltas( pProp, p1, p2 );

	return c1 | c2 | c3;
}

const char* Vector_GetTypeNameString()
{
	return "DPT_Vector";
}


bool Vector_IsZero( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp )
{
	return ( pVar->m_Vector[0] == 0 ) && ( pVar->m_Vector[1] == 0 ) && ( pVar->m_Vector[2] == 0 );
}


void Vector_DecodeZero( DecodeInfo *pInfo )
{
	pInfo->m_Value.m_Vector[0] = 0;
	pInfo->m_Value.m_Vector[1] = 0;
	pInfo->m_Value.m_Vector[2] = 0;

	if ( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}

bool Vector_IsEncodedZero( const SendPropPrecalc *pProp, bf_read *pIn )
{
	float v[3];
	
	DecodeVector( pProp, pIn, v );

	return ( v[0] == 0 ) && ( v[1] == 0 ) && ( v[2] == 0 );
}

void Vector_SkipProp( const SendPropPrecalc *pProp, bf_read *pIn )
{
	Float_SkipProp(pProp, pIn);
	Float_SkipProp(pProp, pIn);
	Float_SkipProp(pProp, pIn);
}

// ---------------------------------------------------------------------------------------- //
// VectorXY type abstraction.
// ---------------------------------------------------------------------------------------- //

void VectorXY_Encode( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp, bf_write *pOut, int objectID )
{
	EncodeFloat(pProp, pVar->m_Vector[0], pOut, objectID);
	EncodeFloat(pProp, pVar->m_Vector[1], pOut, objectID);
}


void VectorXY_Decode(DecodeInfo *pInfo)
{
	pInfo->m_Value.m_Vector[0] = DecodeFloat(pInfo->m_pProp, pInfo->m_pIn);
	pInfo->m_Value.m_Vector[1] = DecodeFloat(pInfo->m_pProp, pInfo->m_pIn);
	
	if( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}


int	VectorXY_CompareDeltas( const SendPropPrecalc *pProp, bf_read *p1, bf_read *p2 )
{
	int c1 = Float_CompareDeltas( pProp, p1, p2 );
	int c2 = Float_CompareDeltas( pProp, p1, p2 );

	return c1 | c2;
}

const char* VectorXY_GetTypeNameString()
{
	return "DPT_VectorXY";
}


bool VectorXY_IsZero( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp )
{
	return ( pVar->m_Vector[0] == 0 ) && ( pVar->m_Vector[1] == 0 );
}


void VectorXY_DecodeZero( DecodeInfo *pInfo )
{
	pInfo->m_Value.m_Vector[0] = 0;
	pInfo->m_Value.m_Vector[1] = 0;
	if ( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}

bool VectorXY_IsEncodedZero( const SendPropPrecalc *pProp, bf_read *pIn )
{
	float v[2];
	
	v[0] = DecodeFloat(pProp, pIn);
	v[1] = DecodeFloat(pProp, pIn);

	return ( v[0] == 0 ) && ( v[1] == 0 );
}

void VectorXY_SkipProp( const SendPropPrecalc *pProp, bf_read *pIn )
{
	Float_SkipProp(pProp, pIn);
	Float_SkipProp(pProp, pIn);
}

// ---------------------------------------------------------------------------------------- //
// String type abstraction.
// ---------------------------------------------------------------------------------------- //

void String_Encode( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp, bf_write *pOut, int objectID )
{
	// First count the string length, then do one WriteBits call.
	int len;
	for ( len=0; len < DT_MAX_STRING_BUFFERSIZE-1; len++ )
	{
		if( pVar->m_pString[len] == 0 )
		{
			break;
		}
	}	
		
	// Optionally write the length here so deltas can be compared faster.
	pOut->WriteUBitLong( len, DT_MAX_STRING_BITS );
	pOut->WriteBits( pVar->m_pString, len * 8 );
}


void String_Decode(DecodeInfo *pInfo)
{
	// Read it in.
	int len = pInfo->m_pIn->ReadUBitLong( DT_MAX_STRING_BITS );

	char *tempStr = pInfo->m_TempStr;

	if ( len >= DT_MAX_STRING_BUFFERSIZE )
	{
		Warning( "String_Decode( %s ) invalid length (%d)\n",
			pInfo->m_pRecvProp ? pInfo->m_pRecvProp->GetName() : "N/A",
			len);
		len = DT_MAX_STRING_BUFFERSIZE - 1;
	}

	pInfo->m_pIn->ReadBits( tempStr, len*8 );
	tempStr[len] = 0;

	pInfo->m_Value.m_pString = tempStr;

	// Give it to the RecvProxy.
	if ( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}


// Compare the bits in pBuf1 and pBuf2 and return 1 if they are different.
// This must always seek both buffers to wherever they start at + nBits.
static inline int AreBitsDifferent( bf_read *pBuf1, bf_read *pBuf2, int nBits )
{
	int nDWords = nBits >> 5;

	int diff = 0;
	for ( int iDWord=0; iDWord < nDWords; iDWord++ )
	{
		diff |= (pBuf1->ReadUBitLong(32) != pBuf2->ReadUBitLong(32));
	}

	int nRemainingBits = nBits - (nDWords<<5);
	if (nRemainingBits > 0)
		diff |= pBuf1->ReadUBitLong( nRemainingBits ) != pBuf2->ReadUBitLong( nRemainingBits );
	
	return diff;
}


int String_CompareDeltas( const SendPropPrecalc *pProp, bf_read *p1, bf_read *p2 )
{
	int len1 = p1->ReadUBitLong( DT_MAX_STRING_BITS );
	int len2 = p2->ReadUBitLong( DT_MAX_STRING_BITS );

	if ( len1 == len2 )
	{
		// check if both strings are empty
		if (len1 == 0)
			return false;

		// Ok, they're short and fast.
		return AreBitsDifferent( p1, p2, len1*8 );
	}
	else
	{
		p1->SeekRelative( len1 * 8 );
		p2->SeekRelative( len2 * 8 );
		return true;
	}
}

const char* String_GetTypeNameString()
{
	return "DPT_String";
}


bool String_IsZero( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp )
{
	return ( pVar->m_pString[0] == 0 );
}


void String_DecodeZero( DecodeInfo *pInfo )
{
	pInfo->m_Value.m_pString = pInfo->m_TempStr;
	pInfo->m_TempStr[0] = 0;
	if ( pInfo->m_pRecvProp )
		pInfo->m_pRecvProp->GetProxyFn()( pInfo, pInfo->m_pStruct, pInfo->m_pData );
}

bool String_IsEncodedZero( const SendPropPrecalc *pProp, bf_read *pIn )
{
	// Read it in.
	int len = pIn->ReadUBitLong( DT_MAX_STRING_BITS );
	
	pIn->SeekRelative( len*8 );

	return len == 0;
}

void String_SkipProp( const SendPropPrecalc *pProp, bf_read *pIn )
{
	int len = pIn->ReadUBitLong( DT_MAX_STRING_BITS );
	pIn->SeekRelative( len*8 );
}


// ---------------------------------------------------------------------------------------- //
// Array abstraction.
// ---------------------------------------------------------------------------------------- //

int Array_GetLength( const unsigned char *pStruct, const SendPropPrecalc *pProp, int objectID )
{
	// Get the array length from the proxy.
	ArrayLengthSendProxyFn proxy = pProp->GetArrayLengthProxy();
	
	if ( proxy )
	{
		int nElements = proxy( pStruct, objectID );
		
		// Make sure it's not too big.
		if ( nElements > pProp->GetNumElements() )
		{
			Assert( false );
			nElements = pProp->GetNumElements();
		}

		return nElements;
	}
	else
	{	
		return pProp->GetNumElements();
	}
}


void Array_Encode( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp, bf_write *pOut, int objectID )
{
	SendProp *pArrayProp = pProp->GetArrayProp();
	AssertMsg( pArrayProp, "Array_Encode: missing m_pArrayProp for SendPropPrecalc '%s'.", pProp->m_pVarName );
	
	int nElements = Array_GetLength( pStruct, pProp, objectID );

	// Write the number of elements.
	pOut->WriteUBitLong( nElements, pProp->GetNumArrayLengthBits() );

	const unsigned char *pCurStructOffset = (const unsigned char*)pStruct + pArrayProp->GetOffset();
	for ( int iElement=0; iElement < nElements; iElement++ )
	{
		DVariant var;

		// Call the proxy to get the value, then encode.
		pArrayProp->GetProxyFn()( pArrayProp, pStruct, pCurStructOffset, &var, iElement, objectID );
		// g_PropTypeFns[pArrayProp->GetType()].Encode( pStruct, &var, pArrayProp, pOut, objectID ); 
		
		pCurStructOffset += pProp->GetElementStride();
	}
}


void Array_Decode( DecodeInfo *pInfo )
{
	SendProp *pArrayProp = pInfo->m_pProp->GetArrayProp();
	AssertMsg( pArrayProp, ("Array_Decode: missing m_pArrayProp for a property.") );

	
}


int Array_CompareDeltas( const SendPropPrecalc *pProp, bf_read *p1, bf_read *p2 )
{
	SendProp *pArrayProp = pProp->GetArrayProp();
	AssertMsg( pArrayProp, "Array_CompareDeltas: missing m_pArrayProp for SendPropPrecalc '%s'.", pProp->m_pVarName );

	int nLengthBits = pProp->GetNumArrayLengthBits(); 
	int length1 = p1->ReadUBitLong( nLengthBits );
	int length2 = p2->ReadUBitLong( nLengthBits );

	int bDifferent = length1 != length2;
	
	return bDifferent;
}


void Array_FastCopy( 
	const SendPropPrecalc *pSendPropPrecalc, 
	const RecvProp *pRecvProp, 
	const unsigned char *pSendData, 
	unsigned char *pRecvData, 
	int objectID )
{

}

const char* Array_GetTypeNameString()
{
	return "DPT_Array";
}


bool Array_IsZero( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp )
{
	int nElements = Array_GetLength( pStruct, pProp, -1 );
	return ( nElements == 0 );
}


void Array_DecodeZero( DecodeInfo *pInfo )
{
	ArrayLengthRecvProxyFn lengthProxy = pInfo->m_pRecvProp->GetArrayLengthProxy();

	if ( lengthProxy )
		lengthProxy( pInfo->m_pStruct, pInfo->m_ObjectID, 0 );
}

bool Array_IsEncodedZero( const SendPropPrecalc *pProp, bf_read *pIn )
{
	SendProp *pArrayProp = pProp->GetArrayProp();
	AssertMsg( pArrayProp, ("Array_IsEncodedZero: missing m_pArrayProp for a property.") );

	int nElements = pIn->ReadUBitLong( pProp->GetNumArrayLengthBits() );

	for ( int i=0; i < nElements;  i++ )
	{
		// skip over data
		// g_PropTypeFns[pArrayProp->GetType()].IsEncodedZero( pArrayProp, pIn );
	}
	
	return nElements == 0;
}

void Array_SkipProp( const SendPropPrecalc *pProp, bf_read *pIn )
{
	SendProp *pArrayProp = pProp->GetArrayProp();
	AssertMsg( pArrayProp, ("Array_SkipProp: missing m_pArrayProp for a property.") );

	int nElements = pIn->ReadUBitLong( pProp->GetNumArrayLengthBits() );

	for ( int i=0; i < nElements;  i++ )
	{
		// skip over data
		// g_PropTypeFns[pArrayProp->GetType()].SkipProp( pArrayProp, pIn );
	}
}


// ---------------------------------------------------------------------------------------- //
// Datatable type abstraction.
// ---------------------------------------------------------------------------------------- //

const char* DataTable_GetTypeNameString()
{
	return "DPT_DataTable";
}


// ---------------------------------------------------------------------------------------- //
// Int 64 property type abstraction.
// ---------------------------------------------------------------------------------------- //

void Int64_Encode( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp, bf_write *pOut, int objectID )
{

}


void Int64_Decode( DecodeInfo *pInfo )
{

}


int Int64_CompareDeltas( const SendPropPrecalc *pProp, bf_read *p1, bf_read *p2 )
{

}

const char* Int64_GetTypeNameString()
{
	return "DPT_Int64";
}


bool Int64_IsZero( const unsigned char *pStruct, DVariant *pVar, const SendPropPrecalc *pProp )
{

}


void Int64_DecodeZero( DecodeInfo *pInfo )
{

}

bool Int64_IsEncodedZero( const SendPropPrecalc *pProp, bf_read *pIn )
{

}

void Int64_SkipProp( const SendPropPrecalc *pProp, bf_read *pIn )
{

}

PropTypeFns g_PropTypeFns[DPT_NUMSendPropTypes] =
{
	// DPT_Int
	{
		Int_Encode,
		Int_Decode,
		Int_CompareDeltas,
		Generic_FastCopy,
		Int_GetTypeNameString,
		Int_IsZero,
		Int_DecodeZero,
		Int_IsEncodedZero,
		Int_SkipProp,
	},

	// DPT_Float
	{
		Float_Encode,
		Float_Decode,
		Float_CompareDeltas,
		Generic_FastCopy,
		Float_GetTypeNameString,
		Float_IsZero,
		Float_DecodeZero,
		Float_IsEncodedZero,
		Float_SkipProp,
	},

	// DPT_Vector
	{
		Vector_Encode,
		Vector_Decode,
		Vector_CompareDeltas,
		Generic_FastCopy,
		Vector_GetTypeNameString,
		Vector_IsZero,
		Vector_DecodeZero,
		Vector_IsEncodedZero,
		Vector_SkipProp,
	},

	// DPT_VectorXY
	{
		VectorXY_Encode,
		VectorXY_Decode,
		VectorXY_CompareDeltas,
		Generic_FastCopy,
		VectorXY_GetTypeNameString,
		VectorXY_IsZero,
		VectorXY_DecodeZero,
		VectorXY_IsEncodedZero,
		VectorXY_SkipProp,
	},

	// DPT_String
	{
		String_Encode,
		String_Decode,
		String_CompareDeltas,
		Generic_FastCopy,
		String_GetTypeNameString,
		String_IsZero,
		String_DecodeZero,
		String_IsEncodedZero,
		String_SkipProp,
	},

	// DPT_Array
	{
		Array_Encode,
		Array_Decode,
		Array_CompareDeltas,
		Array_FastCopy,
		Array_GetTypeNameString,
		Array_IsZero,
		Array_DecodeZero,
		Array_IsEncodedZero,
		Array_SkipProp,
	},
	 
	// DPT_DataTable
	{
		NULL,
		NULL,
		NULL,
		NULL,
		DataTable_GetTypeNameString,
		NULL,
		NULL,
		NULL,
		NULL,
	},
#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!

	// DPT_Quaternion
	{
		Quaternion_Encode,
		Quaternion_Decode,
		Quaternion_CompareDeltas,
		Generic_FastCopy,
		Quaternion_GetTypeNameString,
		Quaternion_IsZero,
		Quaternion_DecodeZero,
		Quaternion_IsEncodedZero,
		Quaternion_SkipProp,
	},
#endif

#ifdef SUPPORTS_INT64
	// DPT_Int64
	{
		Int64_Encode,
		Int64_Decode,
		Int64_CompareDeltas,
		Generic_FastCopy,
		Int64_GetTypeNameString,
		Int64_IsZero,
		Int64_DecodeZero,
		Int64_IsEncodedZero,
		Int64_SkipProp,
	},
#endif

};
