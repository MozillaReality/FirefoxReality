/************************************************************************************

Filename    :   VrApi_Helpers.h
Content     :   Pure, stateless, inlined helper functions, used to initialize
                parameters to the VrApi.
Created     :   March 2, 2015
Authors     :   J.M.P. van Waveren
Language    :   C99

Copyright   :   Copyright 2015 Oculus VR, LLC. All Rights reserved.

*************************************************************************************/
#ifndef OVR_VrApi_Helpers_h
#define OVR_VrApi_Helpers_h

#include "math.h"		// for cosf(), sinf(), tanf()
#include "string.h"		// for memset()
#include "VrApi_Config.h"
#include "VrApi_Version.h"
#include "VrApi_Types.h"

#define VRAPI_PI		3.14159265358979323846f
#define VRAPI_ZNEAR		0.1f

#if defined( __GNUC__ )
#  define   VRAPI_UNUSED(a)   do {__typeof__ (&a) __attribute__ ((unused)) __tmp = &a; } while(0)
#else
#  define   VRAPI_UNUSED(a)   (a)
#endif

//-----------------------------------------------------------------
// Matrix helper functions.
//-----------------------------------------------------------------

static inline ovrVector4f ovrVector4f_MultiplyMatrix4f( const ovrMatrix4f * a, const ovrVector4f * v )
{
	ovrVector4f out;
	out.x = a->M[0][0] * v->x + a->M[0][1] * v->y + a->M[0][2] * v->z + a->M[0][3] * v->w;
	out.y = a->M[1][0] * v->x + a->M[1][1] * v->y + a->M[1][2] * v->z + a->M[1][3] * v->w;
	out.z = a->M[2][0] * v->x + a->M[2][1] * v->y + a->M[2][2] * v->z + a->M[2][3] * v->w;
	out.w = a->M[3][0] * v->x + a->M[3][1] * v->y + a->M[3][2] * v->z + a->M[3][3] * v->w;
	return out;
}

// Use left-multiplication to accumulate transformations.
static inline ovrMatrix4f ovrMatrix4f_Multiply( const ovrMatrix4f * a, const ovrMatrix4f * b )
{
	ovrMatrix4f out;
	out.M[0][0] = a->M[0][0] * b->M[0][0] + a->M[0][1] * b->M[1][0] + a->M[0][2] * b->M[2][0] + a->M[0][3] * b->M[3][0];
	out.M[1][0] = a->M[1][0] * b->M[0][0] + a->M[1][1] * b->M[1][0] + a->M[1][2] * b->M[2][0] + a->M[1][3] * b->M[3][0];
	out.M[2][0] = a->M[2][0] * b->M[0][0] + a->M[2][1] * b->M[1][0] + a->M[2][2] * b->M[2][0] + a->M[2][3] * b->M[3][0];
	out.M[3][0] = a->M[3][0] * b->M[0][0] + a->M[3][1] * b->M[1][0] + a->M[3][2] * b->M[2][0] + a->M[3][3] * b->M[3][0];

	out.M[0][1] = a->M[0][0] * b->M[0][1] + a->M[0][1] * b->M[1][1] + a->M[0][2] * b->M[2][1] + a->M[0][3] * b->M[3][1];
	out.M[1][1] = a->M[1][0] * b->M[0][1] + a->M[1][1] * b->M[1][1] + a->M[1][2] * b->M[2][1] + a->M[1][3] * b->M[3][1];
	out.M[2][1] = a->M[2][0] * b->M[0][1] + a->M[2][1] * b->M[1][1] + a->M[2][2] * b->M[2][1] + a->M[2][3] * b->M[3][1];
	out.M[3][1] = a->M[3][0] * b->M[0][1] + a->M[3][1] * b->M[1][1] + a->M[3][2] * b->M[2][1] + a->M[3][3] * b->M[3][1];

	out.M[0][2] = a->M[0][0] * b->M[0][2] + a->M[0][1] * b->M[1][2] + a->M[0][2] * b->M[2][2] + a->M[0][3] * b->M[3][2];
	out.M[1][2] = a->M[1][0] * b->M[0][2] + a->M[1][1] * b->M[1][2] + a->M[1][2] * b->M[2][2] + a->M[1][3] * b->M[3][2];
	out.M[2][2] = a->M[2][0] * b->M[0][2] + a->M[2][1] * b->M[1][2] + a->M[2][2] * b->M[2][2] + a->M[2][3] * b->M[3][2];
	out.M[3][2] = a->M[3][0] * b->M[0][2] + a->M[3][1] * b->M[1][2] + a->M[3][2] * b->M[2][2] + a->M[3][3] * b->M[3][2];

	out.M[0][3] = a->M[0][0] * b->M[0][3] + a->M[0][1] * b->M[1][3] + a->M[0][2] * b->M[2][3] + a->M[0][3] * b->M[3][3];
	out.M[1][3] = a->M[1][0] * b->M[0][3] + a->M[1][1] * b->M[1][3] + a->M[1][2] * b->M[2][3] + a->M[1][3] * b->M[3][3];
	out.M[2][3] = a->M[2][0] * b->M[0][3] + a->M[2][1] * b->M[1][3] + a->M[2][2] * b->M[2][3] + a->M[2][3] * b->M[3][3];
	out.M[3][3] = a->M[3][0] * b->M[0][3] + a->M[3][1] * b->M[1][3] + a->M[3][2] * b->M[2][3] + a->M[3][3] * b->M[3][3];
	return out;
}

// Returns the transpose of a 4x4 matrix.
static inline ovrMatrix4f ovrMatrix4f_Transpose( const ovrMatrix4f * a )
{
	ovrMatrix4f out;
	out.M[0][0] = a->M[0][0]; out.M[0][1] = a->M[1][0]; out.M[0][2] = a->M[2][0]; out.M[0][3] = a->M[3][0];
	out.M[1][0] = a->M[0][1]; out.M[1][1] = a->M[1][1]; out.M[1][2] = a->M[2][1]; out.M[1][3] = a->M[3][1];
	out.M[2][0] = a->M[0][2]; out.M[2][1] = a->M[1][2]; out.M[2][2] = a->M[2][2]; out.M[2][3] = a->M[3][2];
	out.M[3][0] = a->M[0][3]; out.M[3][1] = a->M[1][3]; out.M[3][2] = a->M[2][3]; out.M[3][3] = a->M[3][3];
	return out;
}

// Returns a 3x3 minor of a 4x4 matrix.
static inline float ovrMatrix4f_Minor( const ovrMatrix4f * m, int r0, int r1, int r2, int c0, int c1, int c2 )
{
	return	m->M[r0][c0] * ( m->M[r1][c1] * m->M[r2][c2] - m->M[r2][c1] * m->M[r1][c2] ) -
			m->M[r0][c1] * ( m->M[r1][c0] * m->M[r2][c2] - m->M[r2][c0] * m->M[r1][c2] ) +
			m->M[r0][c2] * ( m->M[r1][c0] * m->M[r2][c1] - m->M[r2][c0] * m->M[r1][c1] );
}

// Returns the inverse of a 4x4 matrix.
static inline ovrMatrix4f ovrMatrix4f_Inverse( const ovrMatrix4f * m )
{
	const float rcpDet = 1.0f / (	m->M[0][0] * ovrMatrix4f_Minor( m, 1, 2, 3, 1, 2, 3 ) -
									m->M[0][1] * ovrMatrix4f_Minor( m, 1, 2, 3, 0, 2, 3 ) +
									m->M[0][2] * ovrMatrix4f_Minor( m, 1, 2, 3, 0, 1, 3 ) -
									m->M[0][3] * ovrMatrix4f_Minor( m, 1, 2, 3, 0, 1, 2 ) );
	ovrMatrix4f out;
	out.M[0][0] =  ovrMatrix4f_Minor( m, 1, 2, 3, 1, 2, 3 ) * rcpDet;
	out.M[0][1] = -ovrMatrix4f_Minor( m, 0, 2, 3, 1, 2, 3 ) * rcpDet;
	out.M[0][2] =  ovrMatrix4f_Minor( m, 0, 1, 3, 1, 2, 3 ) * rcpDet;
	out.M[0][3] = -ovrMatrix4f_Minor( m, 0, 1, 2, 1, 2, 3 ) * rcpDet;
	out.M[1][0] = -ovrMatrix4f_Minor( m, 1, 2, 3, 0, 2, 3 ) * rcpDet;
	out.M[1][1] =  ovrMatrix4f_Minor( m, 0, 2, 3, 0, 2, 3 ) * rcpDet;
	out.M[1][2] = -ovrMatrix4f_Minor( m, 0, 1, 3, 0, 2, 3 ) * rcpDet;
	out.M[1][3] =  ovrMatrix4f_Minor( m, 0, 1, 2, 0, 2, 3 ) * rcpDet;
	out.M[2][0] =  ovrMatrix4f_Minor( m, 1, 2, 3, 0, 1, 3 ) * rcpDet;
	out.M[2][1] = -ovrMatrix4f_Minor( m, 0, 2, 3, 0, 1, 3 ) * rcpDet;
	out.M[2][2] =  ovrMatrix4f_Minor( m, 0, 1, 3, 0, 1, 3 ) * rcpDet;
	out.M[2][3] = -ovrMatrix4f_Minor( m, 0, 1, 2, 0, 1, 3 ) * rcpDet;
	out.M[3][0] = -ovrMatrix4f_Minor( m, 1, 2, 3, 0, 1, 2 ) * rcpDet;
	out.M[3][1] =  ovrMatrix4f_Minor( m, 0, 2, 3, 0, 1, 2 ) * rcpDet;
	out.M[3][2] = -ovrMatrix4f_Minor( m, 0, 1, 3, 0, 1, 2 ) * rcpDet;
	out.M[3][3] =  ovrMatrix4f_Minor( m, 0, 1, 2, 0, 1, 2 ) * rcpDet;
	return out;
}

// Returns a 4x4 identity matrix.
static inline ovrMatrix4f ovrMatrix4f_CreateIdentity()
{
	ovrMatrix4f out;
	out.M[0][0] = 1.0f; out.M[0][1] = 0.0f; out.M[0][2] = 0.0f; out.M[0][3] = 0.0f;
	out.M[1][0] = 0.0f; out.M[1][1] = 1.0f; out.M[1][2] = 0.0f; out.M[1][3] = 0.0f;
	out.M[2][0] = 0.0f; out.M[2][1] = 0.0f; out.M[2][2] = 1.0f; out.M[2][3] = 0.0f;
	out.M[3][0] = 0.0f; out.M[3][1] = 0.0f; out.M[3][2] = 0.0f; out.M[3][3] = 1.0f;
	return out;
}

// Returns a 4x4 scaling matrix.
static inline ovrMatrix4f ovrMatrix4f_CreateScale( const float x, const float y, const float z )
{
	ovrMatrix4f out;
	out.M[0][0] = x;    out.M[0][1] = 0.0f; out.M[0][2] = 0.0f; out.M[0][3] = 0.0f;
	out.M[1][0] = 0.0f; out.M[1][1] = y;	out.M[1][2] = 0.0f; out.M[1][3] = 0.0f;
	out.M[2][0] = 0.0f; out.M[2][1] = 0.0f; out.M[2][2] = z;    out.M[2][3] = 0.0f;
	out.M[3][0] = 0.0f; out.M[3][1] = 0.0f; out.M[3][2] = 0.0f; out.M[3][3] = 1.0f;
	return out;
}

// Returns a 4x4 homogeneous translation matrix.
static inline ovrMatrix4f ovrMatrix4f_CreateTranslation( const float x, const float y, const float z )
{
	ovrMatrix4f out;
	out.M[0][0] = 1.0f; out.M[0][1] = 0.0f; out.M[0][2] = 0.0f; out.M[0][3] = x;
	out.M[1][0] = 0.0f; out.M[1][1] = 1.0f; out.M[1][2] = 0.0f; out.M[1][3] = y;
	out.M[2][0] = 0.0f; out.M[2][1] = 0.0f; out.M[2][2] = 1.0f; out.M[2][3] = z;
	out.M[3][0] = 0.0f; out.M[3][1] = 0.0f; out.M[3][2] = 0.0f; out.M[3][3] = 1.0f;
	return out;
}

// Returns a 4x4 homogeneous rotation matrix.
static inline ovrMatrix4f ovrMatrix4f_CreateRotation( const float radiansX, const float radiansY, const float radiansZ )
{
	const float sinX = sinf( radiansX );
	const float cosX = cosf( radiansX );
	const ovrMatrix4f rotationX =
	{ {
		{ 1,    0,     0, 0 },
		{ 0, cosX, -sinX, 0 },
		{ 0, sinX,  cosX, 0 },
		{ 0,    0,     0, 1 }
	} };
	const float sinY = sinf( radiansY );
	const float cosY = cosf( radiansY );
	const ovrMatrix4f rotationY =
	{ {
		{  cosY, 0, sinY, 0 },
		{     0, 1,    0, 0 },
		{ -sinY, 0, cosY, 0 },
		{     0, 0,    0, 1 }
	} };
	const float sinZ = sinf( radiansZ );
	const float cosZ = cosf( radiansZ );
	const ovrMatrix4f rotationZ =
	{ {
		{ cosZ, -sinZ, 0, 0 },
		{ sinZ,  cosZ, 0, 0 },
		{    0,     0, 1, 0 },
		{    0,     0, 0, 1 }
	} };
	const ovrMatrix4f rotationXY = ovrMatrix4f_Multiply( &rotationY, &rotationX );
	return ovrMatrix4f_Multiply( &rotationZ, &rotationXY );
}

// Returns a projection matrix based on the specified dimensions.
// The projection matrix transforms -Z=forward, +Y=up, +X=right to the appropriate clip space for the graphics API.
// The far plane is placed at infinity if farZ <= nearZ.
// An infinite projection matrix is preferred for rasterization because, except for
// things *right* up against the near plane, it always provides better precision:
//		"Tightening the Precision of Perspective Rendering"
//		Paul Upchurch, Mathieu Desbrun
//		Journal of Graphics Tools, Volume 16, Issue 1, 2012
static inline ovrMatrix4f ovrMatrix4f_CreateProjection( const float minX, const float maxX,
											float const minY, const float maxY, const float nearZ, const float farZ )
{
	const float width = maxX - minX;
	const float height = maxY - minY;
	const float offsetZ = nearZ;	// set to zero for a [0,1] clip space

	ovrMatrix4f out;
	if ( farZ <= nearZ )
	{
		// place the far plane at infinity
		out.M[0][0] = 2 * nearZ / width;
		out.M[0][1] = 0;
		out.M[0][2] = ( maxX + minX ) / width;
		out.M[0][3] = 0;

		out.M[1][0] = 0;
		out.M[1][1] = 2 * nearZ / height;
		out.M[1][2] = ( maxY + minY ) / height;
		out.M[1][3] = 0;

		out.M[2][0] = 0;
		out.M[2][1] = 0;
		out.M[2][2] = -1;
		out.M[2][3] = -( nearZ + offsetZ );

		out.M[3][0] = 0;
		out.M[3][1] = 0;
		out.M[3][2] = -1;
		out.M[3][3] = 0;
	}
	else
	{
		// normal projection
		out.M[0][0] = 2 * nearZ / width;
		out.M[0][1] = 0;
		out.M[0][2] = ( maxX + minX ) / width;
		out.M[0][3] = 0;

		out.M[1][0] = 0;
		out.M[1][1] = 2 * nearZ / height;
		out.M[1][2] = ( maxY + minY ) / height;
		out.M[1][3] = 0;

		out.M[2][0] = 0;
		out.M[2][1] = 0;
		out.M[2][2] = -( farZ + offsetZ ) / ( farZ - nearZ );
		out.M[2][3] = -( farZ * ( nearZ + offsetZ ) ) / ( farZ - nearZ );

		out.M[3][0] = 0;
		out.M[3][1] = 0;
		out.M[3][2] = -1;
		out.M[3][3] = 0;
	}
	return out;
}

// Returns a projection matrix based on the given FOV.
static inline ovrMatrix4f ovrMatrix4f_CreateProjectionFov( const float fovDegreesX, const float fovDegreesY,
												const float offsetX, const float offsetY, const float nearZ, const float farZ )
{
	const float halfWidth = nearZ * tanf( fovDegreesX * ( VRAPI_PI / 180.0f * 0.5f ) );
	const float halfHeight = nearZ * tanf( fovDegreesY * ( VRAPI_PI / 180.0f * 0.5f ) );

	const float minX = offsetX - halfWidth;
	const float maxX = offsetX + halfWidth;

	const float minY = offsetY - halfHeight;
	const float maxY = offsetY + halfHeight;

	return ovrMatrix4f_CreateProjection( minX, maxX, minY, maxY, nearZ, farZ );
}

// Returns the 4x4 rotation matrix for the given quaternion.
static inline ovrMatrix4f ovrMatrix4f_CreateFromQuaternion( const ovrQuatf * q )
{
	const float ww = q->w * q->w;
	const float xx = q->x * q->x;
	const float yy = q->y * q->y;
	const float zz = q->z * q->z;

	ovrMatrix4f out;
	out.M[0][0] = ww + xx - yy - zz;
	out.M[0][1] = 2 * ( q->x * q->y - q->w * q->z );
	out.M[0][2] = 2 * ( q->x * q->z + q->w * q->y );
	out.M[0][3] = 0;

	out.M[1][0] = 2 * ( q->x * q->y + q->w * q->z );
	out.M[1][1] = ww - xx + yy - zz;
	out.M[1][2] = 2 * ( q->y * q->z - q->w * q->x );
	out.M[1][3] = 0;

	out.M[2][0] = 2 * ( q->x * q->z - q->w * q->y );
	out.M[2][1] = 2 * ( q->y * q->z + q->w * q->x );
	out.M[2][2] = ww - xx - yy + zz;
	out.M[2][3] = 0;

	out.M[3][0] = 0;
	out.M[3][1] = 0;
	out.M[3][2] = 0;
	out.M[3][3] = 1;
	return out;
}

// Convert a standard projection matrix into a TexCoordsFromTanAngles matrix for
// the primary time warp surface.
static inline ovrMatrix4f ovrMatrix4f_TanAngleMatrixFromProjection( const ovrMatrix4f * projection )
{
	/*
		A projection matrix goes from a view point to NDC, or -1 to 1 space.
		Scale and bias to convert that to a 0 to 1 space.

		const ovrMatrix3f m =
		{ {
			{ projection->M[0][0],                0.0f, projection->M[0][2] },
			{                0.0f, projection->M[1][1], projection->M[1][2] },
			{                0.0f,                0.0f,               -1.0f }
		} };
		// Note that there is no Y-flip because eye buffers have 0,0 = left-bottom.
		const ovrMatrix3f s = ovrMatrix3f_CreateScaling( 0.5f, 0.5f );
		const ovrMatrix3f t = ovrMatrix3f_CreateTranslation( 0.5f, 0.5f );
		const ovrMatrix3f r0 = ovrMatrix3f_Multiply( &s, &m );
		const ovrMatrix3f r1 = ovrMatrix3f_Multiply( &t, &r0 );
		return r1;

		clipZ = ( z * projection[2][2] + projection[2][3] ) / ( projection[3][2] * z )
		z = projection[2][3] / ( clipZ * projection[3][2] - projection[2][2] )
		z = ( projection[2][3] / projection[3][2] ) / ( clipZ - projection[2][2] / projection[3][2] )
	*/
	const ovrMatrix4f tanAngleMatrix =
	{ {
		{ 0.5f * projection->M[0][0], 0.0f, 0.5f * projection->M[0][2] - 0.5f, 0.0f },
		{ 0.0f, 0.5f * projection->M[1][1], 0.5f * projection->M[1][2] - 0.5f, 0.0f },
		{ 0.0f, 0.0f, -1.0f, 0.0f },
		// Store the values to convert a clip-Z to a linear depth in the unused matrix elements.
		{ projection->M[2][2], projection->M[2][3], projection->M[3][2], 1.0f }
	} };
	return tanAngleMatrix;
}

// If a simple quad defined as a -1 to 1 XY unit square is transformed to
// the camera view with the given modelView matrix, it can alternately be
// drawn as a time warp overlay image to take advantage of the full window
// resolution, which is usually higher than the eye buffer textures, and
// avoids resampling both into the eye buffer, and again to the screen.
// This is used for high quality movie screens and user interface planes.
//
// Note that this is NOT an MVP matrix -- the "projection" is handled
// by the distortion process.
//
// This utility functions converts a model-view matrix that would normally
// draw a -1 to 1 unit square to the view into a TexCoordsFromTanAngles matrix
// for an overlay surface.
//
// The resulting z value should be straight ahead distance to the plane.
// The x and y values will be pre-multiplied by z for projective texturing.
static inline ovrMatrix4f ovrMatrix4f_TanAngleMatrixFromUnitSquare( const ovrMatrix4f * modelView )
{
	/*
		// Take the inverse of the view matrix because the view matrix transforms the unit square
		// from world space into view space, while the matrix needed here is the one that transforms
		// the unit square from view space to world space.
		const ovrMatrix4f inv = ovrMatrix4f_Inverse( modelView );
		// This matrix calculates the projection onto the (-1, 1) X and Y axes of the unit square,
		// of the intersection of the vector (tanX, tanY, -1) with the plane described by the matrix
		// that transforms the unit square into world space.
		const ovrMatrix3f m =
		{ {
			{	inv.M[0][0] * inv.M[2][3] - inv.M[0][3] * inv.M[2][0],
				inv.M[0][1] * inv.M[2][3] - inv.M[0][3] * inv.M[2][1],
				inv.M[0][2] * inv.M[2][3] - inv.M[0][3] * inv.M[2][2] },
			{	inv.M[1][0] * inv.M[2][3] - inv.M[1][3] * inv.M[2][0],
				inv.M[1][1] * inv.M[2][3] - inv.M[1][3] * inv.M[2][1],
				inv.M[1][2] * inv.M[2][3] - inv.M[1][3] * inv.M[2][2] },
			{	- inv.M[2][0],
				- inv.M[2][1],
				- inv.M[2][2] }
		} };
		// Flip the Y because textures have 0,0 = left-top as opposed to left-bottom.
		const ovrMatrix3f f = ovrMatrix3f_CreateScaling( 1.0f, -1.0f );
		const ovrMatrix3f s = ovrMatrix3f_CreateScaling( 0.5f, 0.5f );
		const ovrMatrix3f t = ovrMatrix3f_CreateTranslation( 0.5f, 0.5f );
		const ovrMatrix3f r0 = ovrMatrix3f_Multiply( &f, &m );
		const ovrMatrix3f r1 = ovrMatrix3f_Multiply( &s, &r0 );
		const ovrMatrix3f r2 = ovrMatrix3f_Multiply( &t, &r1 );
		return r2;
	*/

	const ovrMatrix4f inv = ovrMatrix4f_Inverse( modelView );
	const float coef = ( inv.M[2][3] > 0.0f ) ? 1.0f : -1.0f;

	ovrMatrix4f m;
	m.M[0][0] = ( +0.5f * ( inv.M[0][0] * inv.M[2][3] - inv.M[0][3] * inv.M[2][0] ) - 0.5f * inv.M[2][0] ) * coef;
	m.M[0][1] = ( +0.5f * ( inv.M[0][1] * inv.M[2][3] - inv.M[0][3] * inv.M[2][1] ) - 0.5f * inv.M[2][1] ) * coef;
	m.M[0][2] = ( +0.5f * ( inv.M[0][2] * inv.M[2][3] - inv.M[0][3] * inv.M[2][2] ) - 0.5f * inv.M[2][2] ) * coef;
	m.M[0][3] = 0.0f;

	m.M[1][0] = ( -0.5f * ( inv.M[1][0] * inv.M[2][3] - inv.M[1][3] * inv.M[2][0] ) - 0.5f * inv.M[2][0] ) * coef;
	m.M[1][1] = ( -0.5f * ( inv.M[1][1] * inv.M[2][3] - inv.M[1][3] * inv.M[2][1] ) - 0.5f * inv.M[2][1] ) * coef;
	m.M[1][2] = ( -0.5f * ( inv.M[1][2] * inv.M[2][3] - inv.M[1][3] * inv.M[2][2] ) - 0.5f * inv.M[2][2] ) * coef;
	m.M[1][3] = 0.0f;

	m.M[2][0] = ( -inv.M[2][0] ) * coef;
	m.M[2][1] = ( -inv.M[2][1] ) * coef;
	m.M[2][2] = ( -inv.M[2][2] ) * coef;
	m.M[2][3] = 0.0f;

	m.M[3][0] = 0.0f;
	m.M[3][1] = 0.0f;
	m.M[3][2] = 0.0f;
	m.M[3][3] = 1.0f;
	return m;
}

// Convert a standard view matrix into a TexCoordsFromTanAngles matrix for
// the looking into a cube map.
static inline ovrMatrix4f ovrMatrix4f_TanAngleMatrixForCubeMap( const ovrMatrix4f * viewMatrix )
{
    ovrMatrix4f m = *viewMatrix;
    // clear translation
    for ( int i = 0; i < 3; i++ )
    {
        m.M[ i ][ 3 ] = 0.0f;
    }
    return ovrMatrix4f_Inverse( &m );
}

// Utility function to calculate external velocity for smooth stick yaw turning.
// To reduce judder in FPS style experiences when the application framerate is
// lower than the vsync rate, the rotation from a joypad can be applied to the
// view space distorted eye vectors before applying the time warp.
static inline ovrMatrix4f ovrMatrix4f_CalculateExternalVelocity( const ovrMatrix4f * viewMatrix, const float yawRadiansPerSecond )
{
	const float angle = yawRadiansPerSecond * ( -1.0f / 60.0f );
	const float sinHalfAngle = sinf( angle * 0.5f );
	const float cosHalfAngle = cosf( angle * 0.5f );

	// Yaw is always going to be around the world Y axis
	ovrQuatf quat;
	quat.x = viewMatrix->M[0][1] * sinHalfAngle;
	quat.y = viewMatrix->M[1][1] * sinHalfAngle;
	quat.z = viewMatrix->M[2][1] * sinHalfAngle;
	quat.w = cosHalfAngle;
	return ovrMatrix4f_CreateFromQuaternion( &quat );
}

// Utility function to rotate a point about a pivot
static inline ovrVector3f ovrVector3f_RotateAboutPivot( const ovrQuatf * rotation, const ovrVector3f * pivot, const ovrVector3f * point )
{
	const ovrMatrix4f t0 = ovrMatrix4f_CreateTranslation( pivot->x, pivot->y, pivot->z );
	const ovrMatrix4f r = ovrMatrix4f_CreateFromQuaternion( rotation );
	const ovrMatrix4f t1 = ovrMatrix4f_CreateTranslation( -pivot->x, -pivot->y, -pivot->z );
	const ovrMatrix4f c0 = ovrMatrix4f_Multiply( &t0, &r );
	const ovrMatrix4f c1 = ovrMatrix4f_Multiply( &c0, &t1 );
	const ovrVector4f v = { point->x, point->y, point->z, 1.0f };
	const ovrVector4f v2 = ovrVector4f_MultiplyMatrix4f( &c1, &v );
	const ovrVector3f v3 = { v2.x, v2.y, v2.z };
	return v3;
}

//-----------------------------------------------------------------
// Default initialization helper functions.
//-----------------------------------------------------------------

// Utility function to default initialize the ovrInitParms.
static inline ovrInitParms vrapi_DefaultInitParms( const ovrJava * java )
{
	ovrInitParms parms;
	memset( &parms, 0, sizeof( parms ) );

	parms.Type = VRAPI_STRUCTURE_TYPE_INIT_PARMS;
	parms.ProductVersion = VRAPI_PRODUCT_VERSION;
	parms.MajorVersion = VRAPI_MAJOR_VERSION;
	parms.MinorVersion = VRAPI_MINOR_VERSION;
	parms.PatchVersion = VRAPI_PATCH_VERSION;
	parms.GraphicsAPI = VRAPI_GRAPHICS_API_OPENGL_ES_2;
	parms.Java = *java;

	return parms;
}

// Utility function to default initialize the ovrModeParms.
static inline ovrModeParms vrapi_DefaultModeParms( const ovrJava * java )
{
	ovrModeParms parms;
	memset( &parms, 0, sizeof( parms ) );

	parms.Type = VRAPI_STRUCTURE_TYPE_MODE_PARMS;
	parms.Flags |= VRAPI_MODE_FLAG_ALLOW_POWER_SAVE;
	parms.Flags |= VRAPI_MODE_FLAG_RESET_WINDOW_FULLSCREEN;
	parms.Java = *java;

	return parms;
}

// Utility function to default initialize the ovrPerformanceParms.
static inline ovrPerformanceParms vrapi_DefaultPerformanceParms()
{
	ovrPerformanceParms parms;
	parms.CpuLevel = 2;
	parms.GpuLevel = 2;
	parms.MainThreadTid = 0;
	parms.RenderThreadTid = 0;
	return parms;
}

typedef enum
{
	VRAPI_FRAME_INIT_DEFAULT			= 0,
	VRAPI_FRAME_INIT_BLACK				= 1,
	VRAPI_FRAME_INIT_BLACK_FLUSH		= 2,
	VRAPI_FRAME_INIT_BLACK_FINAL		= 3,
	VRAPI_FRAME_INIT_LOADING_ICON		= 4,
	VRAPI_FRAME_INIT_LOADING_ICON_FLUSH = 5,

	// enum 6 used to be VRAPI_FRAME_INIT_MESSAGE

	// enum 7 used to be VRAPI_FRAME_INIT_MESSAGE_FLUSH
} ovrFrameInit;

// Utility function to default initialize the ovrFrameParms.
static inline ovrFrameParms vrapi_DefaultFrameParms( const ovrJava * java, const ovrFrameInit init, const double currentTime,
													 ovrTextureSwapChain * textureSwapChain )
{
	const ovrMatrix4f projectionMatrix = ovrMatrix4f_CreateProjectionFov( 90.0f, 90.0f, 0.0f, 0.0f, 0.1f, 0.0f );
	const ovrMatrix4f texCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection( &projectionMatrix );

	ovrFrameParms parms;
	memset( &parms, 0, sizeof( parms ) );

	parms.Type = VRAPI_STRUCTURE_TYPE_FRAME_PARMS;
	for ( int layer = 0; layer < VRAPI_FRAME_LAYER_TYPE_MAX; layer++ )
	{
		parms.Layers[layer].ColorScale = 1.0f;
		for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
		{
			parms.Layers[layer].Textures[eye].TexCoordsFromTanAngles = texCoordsFromTanAngles;
			parms.Layers[layer].Textures[eye].TextureRect.width = 1.0f;
			parms.Layers[layer].Textures[eye].TextureRect.height = 1.0f;
			parms.Layers[layer].Textures[eye].HeadPose.Pose.Orientation.w = 1.0f;
			parms.Layers[layer].Textures[eye].HeadPose.TimeInSeconds = currentTime;
		}
	}
	parms.LayerCount = 1;
	parms.SwapInterval = 1;
	parms.ExtraLatencyMode = VRAPI_EXTRA_LATENCY_MODE_OFF;
	parms.ExternalVelocity.M[0][0] = 1.0f;
	parms.ExternalVelocity.M[1][1] = 1.0f;
	parms.ExternalVelocity.M[2][2] = 1.0f;
	parms.ExternalVelocity.M[3][3] = 1.0f;
	parms.PerformanceParms = vrapi_DefaultPerformanceParms();
	parms.Java = *java;

	parms.Layers[0].SrcBlend = VRAPI_FRAME_LAYER_BLEND_ONE;
	parms.Layers[0].DstBlend = VRAPI_FRAME_LAYER_BLEND_ZERO;
	parms.Layers[0].Flags = 0;

	parms.Layers[1].SrcBlend = VRAPI_FRAME_LAYER_BLEND_SRC_ALPHA;
	parms.Layers[1].DstBlend = VRAPI_FRAME_LAYER_BLEND_ONE_MINUS_SRC_ALPHA;
	parms.Layers[1].Flags = 0;

	switch ( init )
	{
		case VRAPI_FRAME_INIT_DEFAULT:
		{
			break;
		}
		case VRAPI_FRAME_INIT_BLACK:
		case VRAPI_FRAME_INIT_BLACK_FLUSH:
		case VRAPI_FRAME_INIT_BLACK_FINAL:
		{
			parms.Flags = VRAPI_FRAME_FLAG_INHIBIT_SRGB_FRAMEBUFFER;
			// NOTE: When requesting a solid black frame, set ColorScale to 0.0f
			parms.Layers[0].ColorScale = 0.0f;
			for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
			{
				parms.Layers[0].Textures[eye].ColorTextureSwapChain = (ovrTextureSwapChain *)VRAPI_DEFAULT_TEXTURE_SWAPCHAIN;
			}
			break;
		}
		case VRAPI_FRAME_INIT_LOADING_ICON:
		case VRAPI_FRAME_INIT_LOADING_ICON_FLUSH:
		{
			parms.LayerCount = 2;
			parms.Flags = VRAPI_FRAME_FLAG_INHIBIT_SRGB_FRAMEBUFFER;
			// NOTE: When requesting a solid black frame, set ColorScale to 0.0f
			parms.Layers[0].ColorScale = 0.0f;
			parms.Layers[1].Flags = VRAPI_FRAME_LAYER_FLAG_SPIN;
			parms.Layers[1].SpinSpeed = 1.0f;		// rotation in radians per second
			parms.Layers[1].SpinScale = 16.0f;		// icon size factor smaller than fullscreen
			for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
			{
				parms.Layers[0].Textures[eye].ColorTextureSwapChain = (ovrTextureSwapChain *)VRAPI_DEFAULT_TEXTURE_SWAPCHAIN;
				parms.Layers[1].Textures[eye].ColorTextureSwapChain = ( textureSwapChain != NULL ) ? textureSwapChain : (ovrTextureSwapChain *)VRAPI_DEFAULT_TEXTURE_SWAPCHAIN_LOADING_ICON;
			}
			break;
		}
	}

	if ( init == VRAPI_FRAME_INIT_BLACK_FLUSH || init == VRAPI_FRAME_INIT_LOADING_ICON_FLUSH )
	{
		parms.Flags |= VRAPI_FRAME_FLAG_FLUSH;
	}
	if ( init == VRAPI_FRAME_INIT_BLACK_FINAL )
	{
		parms.Flags |= VRAPI_FRAME_FLAG_FLUSH | VRAPI_FRAME_FLAG_FINAL;
	}

	return parms;
}

//-----------------------------------------------------------------
// Layer Types - default initialization.
//-----------------------------------------------------------------

static inline ovrLayerProjection2 vrapi_DefaultLayerProjection2()
{
	ovrLayerProjection2 layer = {};

	const ovrMatrix4f projectionMatrix			= ovrMatrix4f_CreateProjectionFov( 90.0f, 90.0f, 0.0f, 0.0f, 0.1f, 0.0f );
	const ovrMatrix4f texCoordsFromTanAngles	= ovrMatrix4f_TanAngleMatrixFromProjection( &projectionMatrix );

	layer.Header.Type	= VRAPI_LAYER_TYPE_PROJECTION2;
	layer.Header.Flags  = 0;
	layer.Header.ColorScale.x	= 1.0f;
	layer.Header.ColorScale.y	= 1.0f;
	layer.Header.ColorScale.z	= 1.0f;
	layer.Header.ColorScale.w	= 1.0f;
	layer.Header.SrcBlend		= VRAPI_FRAME_LAYER_BLEND_ONE;
	layer.Header.DstBlend		= VRAPI_FRAME_LAYER_BLEND_ZERO;
	layer.Header.SurfaceTextureObject = NULL;

	layer.HeadPose.Pose.Orientation.w = 1.0f;

	for ( int i = 0; i < VRAPI_FRAME_LAYER_EYE_MAX; i++ )
	{
		layer.Textures[i].TexCoordsFromTanAngles		= texCoordsFromTanAngles;
		layer.Textures[i].TextureRect.x					= 0.0f;
		layer.Textures[i].TextureRect.y					= 0.0f;
		layer.Textures[i].TextureRect.width				= 1.0f;
		layer.Textures[i].TextureRect.height			= 1.0f;
	}

	return layer;
}

static inline ovrLayerProjection2 vrapi_DefaultLayerBlackProjection2()
{
	ovrLayerProjection2 layer = {};

	layer.Header.Type	= VRAPI_LAYER_TYPE_PROJECTION2;
	layer.Header.Flags  = 0;
	// NOTE: When requesting a solid black frame, set ColorScale to { 0.0f, 0.0f, 0.0f, 0.0f }
	layer.Header.ColorScale.x	= 0.0f;
	layer.Header.ColorScale.y	= 0.0f;
	layer.Header.ColorScale.z	= 0.0f;
	layer.Header.ColorScale.w	= 0.0f;
	layer.Header.SrcBlend		= VRAPI_FRAME_LAYER_BLEND_ONE;
	layer.Header.DstBlend		= VRAPI_FRAME_LAYER_BLEND_ZERO;
	layer.Header.SurfaceTextureObject = NULL;

	layer.HeadPose.Pose.Orientation.w = 1.0f;

	for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
	{
		layer.Textures[eye].SwapChainIndex = 0;
		layer.Textures[eye].ColorSwapChain = (ovrTextureSwapChain *)VRAPI_DEFAULT_TEXTURE_SWAPCHAIN;
	}

	return layer;
}

static inline ovrLayerCylinder2 vrapi_DefaultLayerCylinder2()
{
	ovrLayerCylinder2 layer = {};

	const ovrMatrix4f projectionMatrix			= ovrMatrix4f_CreateProjectionFov( 90.0f, 90.0f, 0.0f, 0.0f, 0.1f, 0.0f );
	const ovrMatrix4f texCoordsFromTanAngles	= ovrMatrix4f_TanAngleMatrixFromProjection( &projectionMatrix );

	layer.Header.Type	= VRAPI_LAYER_TYPE_CYLINDER2;
	layer.Header.Flags  = 0;
	layer.Header.ColorScale.x	= 1.0f;
	layer.Header.ColorScale.y	= 1.0f;
	layer.Header.ColorScale.z	= 1.0f;
	layer.Header.ColorScale.w	= 1.0f;
	layer.Header.SrcBlend		= VRAPI_FRAME_LAYER_BLEND_ONE;
	layer.Header.DstBlend		= VRAPI_FRAME_LAYER_BLEND_ZERO;
	layer.Header.SurfaceTextureObject = NULL;

	layer.HeadPose.Pose.Orientation.w = 1.0f;

	for ( int i = 0; i < VRAPI_FRAME_LAYER_EYE_MAX; i++ )
	{
		layer.Textures[i].TexCoordsFromTanAngles	= texCoordsFromTanAngles;
		layer.Textures[i].TextureRect.x			= 0.0f;
		layer.Textures[i].TextureRect.y			= 0.0f;
		layer.Textures[i].TextureRect.width		= 1.0f;
		layer.Textures[i].TextureRect.height	= 1.0f;
		layer.Textures[i].TextureMatrix.M[0][0] = 1.0f;
		layer.Textures[i].TextureMatrix.M[1][1] = 1.0f;
		layer.Textures[i].TextureMatrix.M[2][2] = 1.0f;
		layer.Textures[i].TextureMatrix.M[3][3] = 1.0f;
	}

	return layer;
}

static inline ovrLayerCube2 vrapi_DefaultLayerCube2()
{
	ovrLayerCube2 layer = {};

	const ovrMatrix4f projectionMatrix			= ovrMatrix4f_CreateProjectionFov( 90.0f, 90.0f, 0.0f, 0.0f, 0.1f, 0.0f );
	const ovrMatrix4f texCoordsFromTanAngles	= ovrMatrix4f_TanAngleMatrixFromProjection( &projectionMatrix );

	layer.Header.Type	= VRAPI_LAYER_TYPE_CUBE2;
	layer.Header.Flags  = 0;
	layer.Header.ColorScale.x	= 1.0f;
	layer.Header.ColorScale.y	= 1.0f;
	layer.Header.ColorScale.z	= 1.0f;
	layer.Header.ColorScale.w	= 1.0f;
	layer.Header.SrcBlend		= VRAPI_FRAME_LAYER_BLEND_ONE;
	layer.Header.DstBlend		= VRAPI_FRAME_LAYER_BLEND_ZERO;
	layer.Header.SurfaceTextureObject = NULL;

	layer.HeadPose.Pose.Orientation.w = 1.0f;
	layer.TexCoordsFromTanAngles = texCoordsFromTanAngles;

	layer.Offset.x = 0.0f;
	layer.Offset.y = 0.0f;
	layer.Offset.z = 0.0f;

	return layer;
}

static inline ovrLayerEquirect2 vrapi_DefaultLayerEquirect2()
{
	ovrLayerEquirect2 layer = {};

	const ovrMatrix4f projectionMatrix			= ovrMatrix4f_CreateProjectionFov( 90.0f, 90.0f, 0.0f, 0.0f, 0.1f, 0.0f );
	const ovrMatrix4f texCoordsFromTanAngles	= ovrMatrix4f_TanAngleMatrixFromProjection( &projectionMatrix );

	layer.Header.Type	= VRAPI_LAYER_TYPE_EQUIRECT2;
	layer.Header.Flags  = 0;
	layer.Header.ColorScale.x	= 1.0f;
	layer.Header.ColorScale.y	= 1.0f;
	layer.Header.ColorScale.z	= 1.0f;
	layer.Header.ColorScale.w	= 1.0f;
	layer.Header.SrcBlend		= VRAPI_FRAME_LAYER_BLEND_ONE;
	layer.Header.DstBlend		= VRAPI_FRAME_LAYER_BLEND_ZERO;
	layer.Header.SurfaceTextureObject = NULL;

	layer.HeadPose.Pose.Orientation.w = 1.0f;
	layer.TexCoordsFromTanAngles = texCoordsFromTanAngles;

	for ( int i = 0; i < VRAPI_FRAME_LAYER_EYE_MAX; i++ )
	{
		layer.Textures[i].TextureRect.x			= 0.0f;
		layer.Textures[i].TextureRect.y			= 0.0f;
		layer.Textures[i].TextureRect.width		= 1.0f;
		layer.Textures[i].TextureRect.height	= 1.0f;
		layer.Textures[i].TextureMatrix.M[0][0] = 1.0f;
		layer.Textures[i].TextureMatrix.M[1][1] = 1.0f;
		layer.Textures[i].TextureMatrix.M[2][2] = 1.0f;
		layer.Textures[i].TextureMatrix.M[3][3] = 1.0f;
	}

	return layer;
}

static inline ovrLayerLoadingIcon2 vrapi_DefaultLayerLoadingIcon2()
{
	ovrLayerLoadingIcon2 layer = {};

	layer.Header.Type	= VRAPI_LAYER_TYPE_LOADING_ICON2;
	layer.Header.Flags  = 0;
	layer.Header.ColorScale.x	= 1.0f;
	layer.Header.ColorScale.y	= 1.0f;
	layer.Header.ColorScale.z	= 1.0f;
	layer.Header.ColorScale.w	= 1.0f;
	layer.Header.SrcBlend		= VRAPI_FRAME_LAYER_BLEND_SRC_ALPHA;
	layer.Header.DstBlend		= VRAPI_FRAME_LAYER_BLEND_ONE_MINUS_SRC_ALPHA;
	layer.Header.SurfaceTextureObject = NULL;

	layer.SpinSpeed			= 1.0f;
	layer.SpinScale			= 16.0f;

	layer.ColorSwapChain	= (ovrTextureSwapChain *)VRAPI_DEFAULT_TEXTURE_SWAPCHAIN_LOADING_ICON;
	layer.SwapChainIndex	= 0;

	return layer;
}


//-----------------------------------------------------------------
// Eye view matrix helper functions.
//-----------------------------------------------------------------

static inline float vrapi_GetInterpupillaryDistance( const ovrTracking2 * tracking2 )
{
	const ovrMatrix4f leftView = tracking2->Eye[0].ViewMatrix;
	const ovrMatrix4f rightView = tracking2->Eye[1].ViewMatrix;
	const ovrVector3f delta = { rightView.M[0][3] - leftView.M[0][3], rightView.M[1][3] - leftView.M[1][3], rightView.M[2][3] - leftView.M[2][3] };
	return sqrtf( delta.x * delta.x + delta.y * delta.y + delta.z * delta.z );
}

static inline float vrapi_GetEyeHeight( const ovrPosef * eyeLevelTrackingPose, const ovrPosef * currentTrackingPose )
{
	return eyeLevelTrackingPose->Position.y - currentTrackingPose->Position.y;
}

static inline ovrMatrix4f vrapi_GetTransformFromPose( const ovrPosef * pose )
{
	const ovrMatrix4f rotation = ovrMatrix4f_CreateFromQuaternion( &pose->Orientation );
	const ovrMatrix4f translation = ovrMatrix4f_CreateTranslation( pose->Position.x, pose->Position.y, pose->Position.z );
	return ovrMatrix4f_Multiply( &translation, &rotation );
}

static inline ovrMatrix4f vrapi_GetViewMatrixFromPose( const ovrPosef * pose )
{
	const ovrMatrix4f transform = vrapi_GetTransformFromPose( pose );
	return ovrMatrix4f_Inverse( &transform );
}

// Utility function to get the eye view matrix based on the center eye view matrix and the IPD.
static inline ovrMatrix4f vrapi_GetEyeViewMatrix(	const ovrMatrix4f * centerEyeViewMatrix,
													const float interpupillaryDistance,
													const int eye )
{
	const float eyeOffset = ( eye ? -0.5f : 0.5f ) * interpupillaryDistance;
	const ovrMatrix4f eyeOffsetMatrix = ovrMatrix4f_CreateTranslation( eyeOffset, 0.0f, 0.0f );
	return ovrMatrix4f_Multiply( &eyeOffsetMatrix, centerEyeViewMatrix );
}

#endif	// OVR_VrApi_Helpers_h
