/*
------------------------------------------------------------------------------
		  Licensing information can be found at the end of the file.
------------------------------------------------------------------------------

vecmath.h - v0.1 - simple vector/matrix math library for C/C++.
*/

#ifndef vecmath_h
#define vecmath_h

// types
typedef struct vec2_t { float x, y; } vec2_t;
typedef struct vec3_t { float x, y, z; } vec3_t;
typedef struct vec4_t { float x, y, z, w; } vec4_t;
typedef struct mat22_t { /* rows */ vec2_t x, y; } mat22_t;
typedef struct mat23_t { /* rows */ vec3_t x, y, z; } mat23_t;
typedef struct mat32_t { /* rows */ vec2_t x, y, z; } mat32_t;
typedef struct mat33_t { /* rows */ vec3_t x, y, z; } mat33_t;
typedef struct mat24_t { /* rows */ vec4_t x, y; } mat24_t;
typedef struct mat34_t { /* rows */ vec4_t x, y, z; } mat34_t;
typedef struct mat42_t { /* rows */ vec2_t x, y, z, w; } mat42_t;
typedef struct mat43_t { /* rows */ vec3_t x, y, z, w; } mat43_t;
typedef struct mat44_t { /* rows */ vec4_t x, y, z, w; } mat44_t;


// functions
float internal_vecmath_abs( float v ) { return (float)( fabsf( v ) ); }
float internal_vecmath_acos( float v ) { return (float)( acosf( v ) ); }
int internal_vecmath_all( float v ) { return v != 0.0f; }
int internal_vecmath_any( float v ) { return v != 0.0f; }
float internal_vecmath_asin( float v ) { return (float)( asinf( v ) ); }
float internal_vecmath_atan( float v ) { return (float)( atanf( v ) ); }
float internal_vecmath_atan2( float y, float x ) { return (float)( atan2f( y, x ) ); }
float internal_vecmath_ceil( float v ) { return (float)( ceilf( v ) ); }
float internal_vecmath_clamp( float v, float internal_vecmath_min, float internal_vecmath_max ) { return (float)( v < internal_vecmath_min ? internal_vecmath_min : v > internal_vecmath_max ? internal_vecmath_max : v ); }
float internal_vecmath_cos( float v ) { return (float)( cosf( v ) ); }
float internal_vecmath_cosh( float v ) { return (float)( coshf( v ) ); }
float internal_vecmath_degrees( float v ) { float const f = 57.295779513082320876846364344191f; return (float)( v * f ); } 
float internal_vecmath_distancesq( float a, float b ) { float x = b - a; return x * x; }
float internal_vecmath_distance( float a, float b ) { float x = b - a; return sqrtf( x * x ); }
float internal_vecmath_dot( float a, float b ) { return a * b;  }
float internal_vecmath_exp( float v ) { return (float)( expf( v ) ); }
float internal_vecmath_exp2( float v ) { return (float)( powf( 2.0f, v ) ); }
float internal_vecmath_floor( float v ) { return (float)( floorf( v ) ); }
float internal_vecmath_fmod( float a, float b ) { return (float)( fmodf( a, b ) ); }
float internal_vecmath_frac( float v ) { float t; return (float)( fabsf( modff( v, &t ) ) ); }
float internal_vecmath_lengthsq( float v ) { return v * v; }
float internal_vecmath_length( float v ) { return sqrtf( v * v ); }
float internal_vecmath_lerp( float a, float b, float s ) { return (float)( a + ( b - a ) * s ); }
float internal_vecmath_log( float v ) { return (float)( logf( v ) ); }
float internal_vecmath_log2( float v ) { return (float)( log2f( v ) ); }
float internal_vecmath_log10( float v ) { return (float)( log10f( v ) ); }
float internal_vecmath_max( float a, float b ) { return (float)( a > b ? a : b ); }
float internal_vecmath_min( float a, float b ) { return (float)( a < b ? a : b ); }
float internal_vecmath_normalize( float v ) { float l = sqrtf( v * v ); return l == 0.0f ? v : (float)( v / l ); }
float internal_vecmath_pow( float a, float b ) { return (float)( powf( a, b ) ); }
float internal_vecmath_radians( float v ) { float const f = 0.01745329251994329576922222222222f; return (float)( v * f ); } 
float internal_vecmath_rcp( float v ) { return (float)( 1.0f / v ); }
float internal_vecmath_reflect( float i, float n ) {  return i - 2.0f * n * internal_vecmath_dot( i, n ); }
float internal_vecmath_refract( float i, float n, float r ) { float n_i = internal_vecmath_dot( n, i ); float k = 1.0f - r * r * ( 1.0f - n_i * n_i ); return ( k < 0.0f ) ? (float)( 0.0f ) : ( r * i - ( r * n_i + sqrtf( k ) ) * n ); }
float internal_vecmath_round( float v ) { return (float)( roundf( v ) ); }
float internal_vecmath_rsqrt( float v ) { return (float)( 1.0f / sqrtf( v ) ); }
float internal_vecmath_saturate( float v ) { return (float)( v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v ); }
float internal_vecmath_sign( float v ) { return (float)( v < 0.0f ? -1.0f : v > 0.0f ? 1.0f : 0.0f ); }
float internal_vecmath_sin( float v ) { return (float)( sinf( v ) ); }
float internal_vecmath_sinh( float v ) { return (float)( sinhf( v ) ); }
float internal_vecmath_smoothstep( float internal_vecmath_min, float internal_vecmath_max, float v ) { v = ( v - internal_vecmath_min ) / ( internal_vecmath_max - internal_vecmath_min ); v = (float)( v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v ); return (float)( v * v * ( 3.0f - 2.0f * v ) ); }
float internal_vecmath_smootherstep( float internal_vecmath_min, float internal_vecmath_max, float v ) { v = ( v - internal_vecmath_min ) / ( internal_vecmath_max - internal_vecmath_min ); v = (float)( v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v ); return (float)( v * v * v * ( v * ( v * 6.0f - 15.0f ) + 10.0f ) ); }
float internal_vecmath_sqrt( float v ) { return (float)( sqrtf( v ) ); }
float internal_vecmath_step( float a, float b ) { return (float)( b >= a ? 1.0f : 0.0f ); }
float internal_vecmath_tan( float v ) { return (float)( tanf( v ) ); }
float internal_vecmath_tanh( float v ) { return (float)( tanhf( v ) ); }
float internal_vecmath_trunc( float v ) { return (float)( truncf( v ) ); }


// vec2
vec2_t vec2( float x, float y ) { vec2_t vec; vec.x = x; vec.y = y; return vec; }
vec2_t vec2f( float v ) { vec2_t vec; vec.x = v; vec.y = v; return vec; }
	
float vec2_get( vec2_t vec, int index ) { return ( (float*) &vec )[ index ]; }
void vec2_set( vec2_t vec, int index, float value ) { ( (float*) &vec )[ index ] = value; }
	

// operators
vec2_t vec2_neg( vec2_t v ) { return vec2( -v.x, -v.y ); }
int vec2_eq( vec2_t a, vec2_t b ) { return a.x == b.x && a.y == b.y; }
vec2_t vec2_add( vec2_t a, vec2_t b ) { return vec2( a.x + b.x, a.y + b.y ); }
vec2_t vec2_sub( vec2_t a, vec2_t b ) { return vec2( a.x - b.x, a.y - b.y ); }
vec2_t vec2_mul( vec2_t a, vec2_t b ) { return vec2( a.x * b.x, a.y * b.y ); }
vec2_t vec2_div( vec2_t a, vec2_t b ) { return vec2( a.x / b.x, a.y / b.y ); }
vec2_t vec2_addf( vec2_t a, float s ) { return vec2( a.x + s, a.y + s ); }
vec2_t vec2_subf( vec2_t a, float s ) { return vec2( a.x - s, a.y - s ); }
vec2_t vec2_mulf( vec2_t a, float s ) { return vec2( a.x * s, a.y * s ); }
vec2_t vec2_divf( vec2_t a, float s ) { return vec2( a.x / s, a.y / s ); }

// functions
vec2_t vec2_abs( vec2_t v ) { return vec2( internal_vecmath_abs( v.x ), internal_vecmath_abs( v.y ) ); }
vec2_t vec2_acos( vec2_t v ) { return vec2( internal_vecmath_acos( v.x ), internal_vecmath_acos( v.y ) ); }
int vec2_all( vec2_t v ) { return v.x != 0.0f && v.y != 0.0f; }
int vec2_any( vec2_t v ) { return v.x != 0.0f || v.y != 0.0f; }
vec2_t vec2_asin( vec2_t v ) { return vec2( internal_vecmath_asin( v.x ), internal_vecmath_asin( v.y ) ); }
vec2_t vec2_atan( vec2_t v ) { return vec2( internal_vecmath_atan( v.x ), internal_vecmath_atan( v.y ) ); }
vec2_t vec2_atan2( vec2_t y, vec2_t x ) { return vec2( internal_vecmath_atan2( y.x, x.x ), internal_vecmath_atan2( y.y, x.y ) ); }
vec2_t vec2_ceil( vec2_t v ) { return vec2( internal_vecmath_ceil( v.x ), internal_vecmath_ceil( v.y ) ); }
vec2_t vec2_clamp( vec2_t v, vec2_t internal_vecmath_min, vec2_t internal_vecmath_max ) { return vec2( internal_vecmath_clamp( v.x, internal_vecmath_min.x, internal_vecmath_max.x ), internal_vecmath_clamp( v.y, internal_vecmath_min.y, internal_vecmath_max.y ) ); }
vec2_t vec2_cos( vec2_t v ) { return vec2( internal_vecmath_cos( v.x ), internal_vecmath_cos( v.y ) ); }
vec2_t vec2_cosh( vec2_t v ) { return vec2( internal_vecmath_cosh( v.x ), internal_vecmath_cosh( v.y ) ); }
vec2_t vec2_degrees( vec2_t v ) { return vec2( internal_vecmath_degrees( v.x ), internal_vecmath_degrees( v.y ) ); } 
float vec2_distancesq( vec2_t a, vec2_t b ) { float x = b.x - a.x; float y = b.y - a.y; return x * x + y * y; }
float vec2_distance( vec2_t a, vec2_t b ) { float x = b.x - a.x; float y = b.y - a.y; return internal_vecmath_sqrt( x * x + y * y ); }
float vec2_dot( vec2_t a, vec2_t b ) { return a.x * b.x + a.y * b.y;  }
vec2_t vec2_exp( vec2_t v ) { return vec2( internal_vecmath_exp( v.x ), internal_vecmath_exp( v.y ) ); }
vec2_t vec2_exp2( vec2_t v ) { return vec2( internal_vecmath_exp2( v.x ), internal_vecmath_exp2( v.y ) ); }
vec2_t vec2_floor( vec2_t v ) { return vec2( internal_vecmath_floor( v.x ), internal_vecmath_floor( v.y ) ); }
vec2_t vec2_fmod( vec2_t a, vec2_t b ) { return vec2( internal_vecmath_fmod( a.x, b.x ), internal_vecmath_fmod( a.y, b.y ) ); }
vec2_t vec2_frac( vec2_t v ) { return vec2( internal_vecmath_frac( v.x ), internal_vecmath_frac( v.y ) ); }
float vec2_lengthsq( vec2_t v ) { return v.x * v.x + v.y * v.y; }
float vec2_length( vec2_t v ) { return internal_vecmath_sqrt( v.x * v.x + v.y * v.y ); }
vec2_t vec2_lerp( vec2_t a, vec2_t b, float s ) { return vec2( internal_vecmath_lerp( a.x, b.x, s ), internal_vecmath_lerp( a.y, b.y, s ) ); }
vec2_t vec2_log( vec2_t v ) { return vec2( internal_vecmath_log( v.x ), internal_vecmath_log( v.y ) ); }
vec2_t vec2_log2( vec2_t v ) { return vec2( internal_vecmath_log2( v.x ), internal_vecmath_log2( v.y ) ); }
vec2_t vec2_log10( vec2_t v ) { return vec2( internal_vecmath_log10( v.x ), internal_vecmath_log10( v.y ) ); }
vec2_t vec2_max( vec2_t a, vec2_t b ) { return vec2( internal_vecmath_max( a.x, b.x ), internal_vecmath_max( a.y, b.y ) ); }
vec2_t vec2_min( vec2_t a, vec2_t b ) { return vec2( internal_vecmath_min( a.x, b.x ), internal_vecmath_min( a.y, b.y ) ); }
vec2_t vec2_normalize( vec2_t v ) { float l = internal_vecmath_sqrt( v.x * v.x + v.y * v.y ); return l == 0.0f ? v : vec2( v.x / l, v.y / l ); }
vec2_t vec2_pow( vec2_t a, vec2_t b ) { return vec2( internal_vecmath_pow( a.x, b.x ), internal_vecmath_pow( a.y, b.y ) ); }
vec2_t vec2_radians( vec2_t v ) { return vec2( internal_vecmath_radians( v.x ), internal_vecmath_radians( v.y ) ); } 
vec2_t vec2_rcp( vec2_t v ) { return vec2( internal_vecmath_rcp( v.x ), internal_vecmath_rcp( v.y ) ); }
vec2_t vec2_round( vec2_t v ) { return vec2( internal_vecmath_round( v.x ), internal_vecmath_round( v.y ) ); }
vec2_t vec2_rsqrt( vec2_t v ) { return vec2( internal_vecmath_rcp( internal_vecmath_sqrt( v.x ) ), internal_vecmath_rcp( internal_vecmath_sqrt( v.y ) ) ); }
vec2_t vec2_saturate( vec2_t v ) { return vec2( internal_vecmath_saturate( v.x ), internal_vecmath_saturate( v.y ) ); }
vec2_t vec2_sign( vec2_t v ) { return vec2( internal_vecmath_sign( v.x ), internal_vecmath_sign( v.y ) ); }
vec2_t vec2_sin( vec2_t v ) { return vec2( internal_vecmath_sin( v.x ), internal_vecmath_sin( v.y ) ); }
vec2_t vec2_sinh( vec2_t v ) { return vec2( internal_vecmath_sinh( v.x ), internal_vecmath_sinh( v.y ) ); }
vec2_t vec2_smoothstep( vec2_t internal_vecmath_min, vec2_t internal_vecmath_max, vec2_t v ) { return vec2( internal_vecmath_smoothstep( internal_vecmath_min.x, internal_vecmath_max.x, v.x ), internal_vecmath_smoothstep( internal_vecmath_min.y, internal_vecmath_max.y, v.y ) ); }
vec2_t vec2_smootherstep( vec2_t internal_vecmath_min, vec2_t internal_vecmath_max, vec2_t v ) { return vec2( internal_vecmath_smootherstep( internal_vecmath_min.x, internal_vecmath_max.x, v.x ), internal_vecmath_smootherstep( internal_vecmath_min.y, internal_vecmath_max.y, v.y ) ); }
vec2_t vec2_sqrt( vec2_t v ) { return vec2( internal_vecmath_sqrt( v.x ), internal_vecmath_sqrt( v.y ) ); }
vec2_t vec2_step( vec2_t a, vec2_t b ) { return vec2( internal_vecmath_step( a.x, b.x ), internal_vecmath_step( a.y, b.y ) ); }
vec2_t vec2_tan( vec2_t v ) { return vec2( internal_vecmath_tan( v.x ), internal_vecmath_tan( v.y ) ); }
vec2_t vec2_tanh( vec2_t v ) { return vec2( internal_vecmath_tanh( v.x ), internal_vecmath_tanh( v.y ) ); }
vec2_t vec2_trunc( vec2_t v ) { return vec2( internal_vecmath_trunc( v.x ), internal_vecmath_trunc( v.y ) ); }


// vec3
vec3_t vec3( float x, float y, float z ) { vec3_t vec; vec.x = x; vec.y = y; vec.z = z; return vec; }
vec3_t vec3f( float v ) { vec3_t vec; vec.x = v; vec.y = v; vec.z = v; return vec; }
vec3_t vec3v2f( vec2_t v, float f ) { vec3_t vec; vec.x = v.x; vec.y = v.y; vec.z = f; return vec; }
vec3_t vec3fv2( float f, vec2_t v ) { vec3_t vec; vec.x = f; vec.y = v.x; vec.z = v.y; return vec; }
	
float vec3_get( vec3_t vec, int index ) { return ( (float*) &vec )[ index ]; }
void vec3_set( vec3_t vec, int index, float value ) { ( (float*) &vec )[ index ] = value; }

// operators
vec3_t vec3_neg( vec3_t v ) { return vec3( -v.x, -v.y, -v.z ); }
int vec3_eq( vec3_t a, vec3_t b ) { return a.x == b.x && a.y == b.y && a.z == b.z; }
vec3_t vec3_add( vec3_t a, vec3_t b ) { return vec3( a.x + b.x, a.y + b.y, a.z + b.z ); }
vec3_t vec3_sub( vec3_t a, vec3_t b ) { return vec3( a.x - b.x, a.y - b.y, a.z - b.z ); }
vec3_t vec3_mul( vec3_t a, vec3_t b ) { return vec3( a.x * b.x, a.y * b.y, a.z * b.z ); }
vec3_t vec3_div( vec3_t a, vec3_t b ) { return vec3( a.x / b.x, a.y / b.y, a.z / b.z ); }
vec3_t vec3_addf( vec3_t a, float s ) { return vec3( a.x + s, a.y + s, a.z + s ); }
vec3_t vec3_subf( vec3_t a, float s ) { return vec3( a.x - s, a.y - s, a.z - s ); }
vec3_t vec3_mulf( vec3_t a, float s ) { return vec3( a.x * s, a.y * s, a.z * s ); }
vec3_t vec3_divf( vec3_t a, float s ) { return vec3( a.x / s, a.y / s, a.z / s ); }

// functions
vec3_t vec3_abs( vec3_t v ) { return vec3( internal_vecmath_abs( v.x ), internal_vecmath_abs( v.y ), internal_vecmath_abs( v.z ) ); }
vec3_t vec3_acos( vec3_t v ) { return vec3( internal_vecmath_acos( v.x ), internal_vecmath_acos( v.y ), internal_vecmath_acos( v.z ) ); }
int vec3_all( vec3_t v ) { return v.x != 0.0f && v.y != 0.0f && v.z != 0.0f; }
int vec3_any( vec3_t v ) { return v.x != 0.0f || v.y != 0.0f || v.z != 0.0f; }
vec3_t vec3_asin( vec3_t v ) { return vec3( internal_vecmath_asin( v.x ), internal_vecmath_asin( v.y ), internal_vecmath_asin( v.z ) ); }
vec3_t vec3_atan( vec3_t v ) { return vec3( internal_vecmath_atan( v.x ), internal_vecmath_atan( v.y ), internal_vecmath_atan( v.z ) ); }
vec3_t vec3_atan2( vec3_t y, vec3_t x ) { return vec3( internal_vecmath_atan2( y.x, x.x ), internal_vecmath_atan2( y.y, x.y ), internal_vecmath_atan2( y.z, x.z ) ); }
vec3_t vec3_ceil( vec3_t v ) { return vec3( internal_vecmath_ceil( v.x ), internal_vecmath_ceil( v.y ), internal_vecmath_ceil( v.z ) ); }
vec3_t vec3_clamp( vec3_t v, vec3_t internal_vecmath_min, vec3_t internal_vecmath_max ) { return vec3( internal_vecmath_clamp( v.x, internal_vecmath_min.x, internal_vecmath_max.x ), internal_vecmath_clamp( v.y, internal_vecmath_min.y, internal_vecmath_max.y ), internal_vecmath_clamp( v.z, internal_vecmath_min.z, internal_vecmath_max.z ) ); }
vec3_t vec3_cos( vec3_t v ) { return vec3( internal_vecmath_cos( v.x ), internal_vecmath_cos( v.y ), internal_vecmath_cos( v.z ) ); }
vec3_t vec3_cosh( vec3_t v ) { return vec3( internal_vecmath_cosh( v.x ), internal_vecmath_cosh( v.y ), internal_vecmath_cosh( v.z ) ); }
vec3_t vec3_cross( vec3_t a, vec3_t b ) { return vec3( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x ); }
vec3_t vec3_degrees( vec3_t v ) { return vec3( internal_vecmath_degrees( v.x ), internal_vecmath_degrees( v.y ), internal_vecmath_degrees( v.z ) ); } 
float vec3_distancesq( vec3_t a, vec3_t b ) { float x = b.x - a.x; float y = b.y - a.y; float z = b.z - a.z; return x * x + y * y + z * z; }
float vec3_distance( vec3_t a, vec3_t b ) { float x = b.x - a.x; float y = b.y - a.y; float z = b.z - a.z; return internal_vecmath_sqrt( x * x + y * y + z * z ); }
float vec3_dot( vec3_t a, vec3_t b ) { return a.x * b.x + a.y * b.y + a.z * b.z;  }
vec3_t vec3_exp( vec3_t v ) { return vec3( internal_vecmath_exp( v.x ), internal_vecmath_exp( v.y ), internal_vecmath_exp( v.z ) ); }
vec3_t vec3_exp2( vec3_t v ) { return vec3( internal_vecmath_exp2( v.x ), internal_vecmath_exp2( v.y ), internal_vecmath_exp2( v.z ) ); }
vec3_t vec3_floor( vec3_t v ) { return vec3( internal_vecmath_floor( v.x ), internal_vecmath_floor( v.y ), internal_vecmath_floor( v.z ) ); }
vec3_t vec3_fmod( vec3_t a, vec3_t b ) { return vec3( internal_vecmath_fmod( a.x, b.x ), internal_vecmath_fmod( a.y, b.y ), internal_vecmath_fmod( a.z, b.z ) ); }
vec3_t vec3_frac( vec3_t v ) { return vec3( internal_vecmath_frac( v.x ), internal_vecmath_frac( v.y ), internal_vecmath_frac( v.z ) ); }
float vec3_lengthsq( vec3_t v ) { return v.x * v.x + v.y * v.y + v.z * v.z; }
float vec3_length( vec3_t v ) { return internal_vecmath_sqrt( v.x * v.x + v.y * v.y + v.z * v.z ); }
vec3_t vec3_lerp( vec3_t a, vec3_t b, float s ) { return vec3( internal_vecmath_lerp( a.x, b.x, s ), internal_vecmath_lerp( a.y, b.y, s ), internal_vecmath_lerp( a.z, b.z, s ) ); }
vec3_t vec3_log( vec3_t v ) { return vec3( internal_vecmath_log( v.x ), internal_vecmath_log( v.y ), internal_vecmath_log( v.z ) ); }
vec3_t vec3_log2( vec3_t v ) { return vec3( internal_vecmath_log2( v.x ), internal_vecmath_log2( v.y ), internal_vecmath_log2( v.z ) ); }
vec3_t vec3_log10( vec3_t v ) { return vec3( internal_vecmath_log10( v.x ), internal_vecmath_log10( v.y ), internal_vecmath_log10( v.z ) ); }
vec3_t vec3_max( vec3_t a, vec3_t b ) { return vec3( internal_vecmath_max( a.x, b.x ), internal_vecmath_max( a.y, b.y ), internal_vecmath_max( a.z, b.z ) ); }
vec3_t vec3_min( vec3_t a, vec3_t b ) { return vec3( internal_vecmath_min( a.x, b.x ), internal_vecmath_min( a.y, b.y ), internal_vecmath_min( a.z, b.z ) ); }
vec3_t vec3_normalize( vec3_t v ) { float l = internal_vecmath_sqrt( v.x * v.x + v.y * v.y + v.z * v.z ); return l == 0.0f ? v : vec3( v.x / l, v.y / l, v.z / l ); }
vec3_t vec3_pow( vec3_t a, vec3_t b ) { return vec3( internal_vecmath_pow( a.x, b.x ), internal_vecmath_pow( a.y, b.y ), internal_vecmath_pow( a.z, b.z ) ); }
vec3_t vec3_radians( vec3_t v ) { return vec3( internal_vecmath_radians( v.x ), internal_vecmath_radians( v.y ), internal_vecmath_radians( v.z ) ); } 
vec3_t vec3_rcp( vec3_t v ) { return vec3( internal_vecmath_rcp( v.x ), internal_vecmath_rcp( v.y ), internal_vecmath_rcp( v.z ) ); }
vec3_t vec3_round( vec3_t v ) { return vec3( internal_vecmath_round( v.x ), internal_vecmath_round( v.y ), internal_vecmath_round( v.z ) ); }
vec3_t vec3_rsqrt( vec3_t v ) { return vec3( internal_vecmath_rcp( internal_vecmath_sqrt( v.x ) ), internal_vecmath_rcp( internal_vecmath_sqrt( v.y ) ), internal_vecmath_rcp( internal_vecmath_sqrt( v.z ) ) ); }
vec3_t vec3_saturate( vec3_t v ) { return vec3( internal_vecmath_saturate( v.x ), internal_vecmath_saturate( v.y ), internal_vecmath_saturate( v.z ) ); }
vec3_t vec3_sign( vec3_t v ) { return vec3( internal_vecmath_sign( v.x ), internal_vecmath_sign( v.y ), internal_vecmath_sign( v.z ) ); }
vec3_t vec3_sin( vec3_t v ) { return vec3( internal_vecmath_sin( v.x ), internal_vecmath_sin( v.y ), internal_vecmath_sin( v.z ) ); }
vec3_t vec3_sinh( vec3_t v ) { return vec3( internal_vecmath_sinh( v.x ), internal_vecmath_sinh( v.y ), internal_vecmath_sinh( v.z ) ); }
vec3_t vec3_smoothstep( vec3_t internal_vecmath_min, vec3_t internal_vecmath_max, vec3_t v ) { return vec3( internal_vecmath_smoothstep( internal_vecmath_min.x, internal_vecmath_max.x, v.x ), internal_vecmath_smoothstep( internal_vecmath_min.y, internal_vecmath_max.y, v.y ), internal_vecmath_smoothstep( internal_vecmath_min.z, internal_vecmath_max.z, v.z ) ); }
vec3_t vec3_smootherstep( vec3_t internal_vecmath_min, vec3_t internal_vecmath_max, vec3_t v ) { return vec3( internal_vecmath_smootherstep( internal_vecmath_min.x, internal_vecmath_max.x, v.x ), internal_vecmath_smootherstep( internal_vecmath_min.y, internal_vecmath_max.y, v.y ), internal_vecmath_smootherstep( internal_vecmath_min.z, internal_vecmath_max.z, v.z ) ); }
vec3_t vec3_sqrt( vec3_t v ) { return vec3( internal_vecmath_sqrt( v.x ), internal_vecmath_sqrt( v.y ), internal_vecmath_sqrt( v.z ) ); }
vec3_t vec3_step( vec3_t a, vec3_t b ) { return vec3( internal_vecmath_step( a.x, b.x ), internal_vecmath_step( a.y, b.y ), internal_vecmath_step( a.z, b.z ) ); }
vec3_t vec3_tan( vec3_t v ) { return vec3( internal_vecmath_tan( v.x ), internal_vecmath_tan( v.y ), internal_vecmath_tan( v.z ) ); }
vec3_t vec3_tanh( vec3_t v ) { return vec3( internal_vecmath_tanh( v.x ), internal_vecmath_tanh( v.y ), internal_vecmath_tanh( v.z ) ); }
vec3_t vec3_trunc( vec3_t v ) { return vec3( internal_vecmath_trunc( v.x ), internal_vecmath_trunc( v.y ), internal_vecmath_trunc( v.z ) ); }

// vec4
vec4_t vec4( float x, float y, float z, float w ) { vec4_t vec; vec.x = x; vec.y = y; vec.z = z; vec.w = w; return vec; }
vec4_t vec4f( float v ) { vec4_t vec; vec.x = v; vec.y = v; vec.z = v; vec.w = v; return vec; }
vec4_t vec4v2( vec2_t a, vec2_t b ) { vec4_t vec; vec.x = a.x; vec.y = a.y; vec.z = b.x; vec.w = b.y; return vec; }
vec4_t vec4v3f( vec3_t v, float f ) { vec4_t vec; vec.x = v.x; vec.y = v.y; vec.z = v.z; vec.w = f; return vec; }
vec4_t vec4fv3( float f, vec3_t v ) { vec4_t vec; vec.x = f; vec.y = v.x; vec.z = v.y; vec.w = v.z; return vec; }
	
float vec4_get( vec4_t vec, int index ) { return ( (float*) &vec )[ index ]; }
void vec4_set( vec4_t vec, int index, float value ) { ( (float*) &vec )[ index ] = value; }

// operators
vec4_t vec4_neg( vec4_t v ) { return vec4( -v.x, -v.y, -v.z, -v.w ); }
int vec4_eq( vec4_t a, vec4_t b ) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
vec4_t vec4_add( vec4_t a, vec4_t b ) { return vec4( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w ); }
vec4_t vec4_sub( vec4_t a, vec4_t b ) { return vec4( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w ); }
vec4_t vec4_mul( vec4_t a, vec4_t b ) { return vec4( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w ); }
vec4_t vec4_div( vec4_t a, vec4_t b ) { return vec4( a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w ); }
vec4_t vec4_addf( vec4_t a, float s ) { return vec4( a.x + s, a.y + s, a.z + s, a.w + s ); }
vec4_t vec4_subf( vec4_t a, float s ) { return vec4( a.x - s, a.y - s, a.z - s, a.w - s ); }
vec4_t vec4_mulf( vec4_t a, float s ) { return vec4( a.x * s, a.y * s, a.z * s, a.w * s ); }
vec4_t vec4_divf( vec4_t a, float s ) { return vec4( a.x / s, a.y / s, a.z / s, a.w / s ); }

// functions
vec4_t vec4_abs( vec4_t v ) { return vec4( internal_vecmath_abs( v.x ), internal_vecmath_abs( v.y ), internal_vecmath_abs( v.z ), internal_vecmath_abs( v.w ) ); }
vec4_t vec4_acos( vec4_t v ) { return vec4( internal_vecmath_acos( v.x ), internal_vecmath_acos( v.y ), internal_vecmath_acos( v.z ), internal_vecmath_acos( v.w ) ); }
int vec4_all( vec4_t v ) { return v.x != 0.0f && v.y != 0.0f && v.z != 0.0f && v.w != 0.0f; }
int vec4_any( vec4_t v ) { return v.x != 0.0f || v.y != 0.0f || v.z != 0.0f || v.w != 0.0f; }
vec4_t vec4_asin( vec4_t v ) { return vec4( internal_vecmath_asin( v.x ), internal_vecmath_asin( v.y ), internal_vecmath_asin( v.z ), internal_vecmath_asin( v.w ) ); }
vec4_t vec4_atan( vec4_t v ) { return vec4( internal_vecmath_atan( v.x ), internal_vecmath_atan( v.y ), internal_vecmath_atan( v.z ), internal_vecmath_atan( v.w ) ); }
vec4_t vec4_atan2( vec4_t y, vec4_t x ) { return vec4( internal_vecmath_atan2( y.x, x.x ), internal_vecmath_atan2( y.y, x.y ), internal_vecmath_atan2( y.z, x.z ), internal_vecmath_atan2( y.w, x.w ) ); }
vec4_t vec4_ceil( vec4_t v ) { return vec4( internal_vecmath_ceil( v.x ), internal_vecmath_ceil( v.y ), internal_vecmath_ceil( v.z ), internal_vecmath_ceil( v.w ) ); }
vec4_t vec4_clamp( vec4_t v, vec4_t internal_vecmath_min, vec4_t internal_vecmath_max ) { return vec4( internal_vecmath_clamp( v.x, internal_vecmath_min.x, internal_vecmath_max.x ), internal_vecmath_clamp( v.y, internal_vecmath_min.y, internal_vecmath_max.y ), internal_vecmath_clamp( v.z, internal_vecmath_min.z, internal_vecmath_max.z ), internal_vecmath_clamp( v.w, internal_vecmath_min.w, internal_vecmath_max.w ) ); }
vec4_t vec4_cos( vec4_t v ) { return vec4( internal_vecmath_cos( v.x ), internal_vecmath_cos( v.y ), internal_vecmath_cos( v.z ), internal_vecmath_cos( v.w ) ); }
vec4_t vec4_cosh( vec4_t v ) { return vec4( internal_vecmath_cosh( v.x ), internal_vecmath_cosh( v.y ), internal_vecmath_cosh( v.z ), internal_vecmath_cosh( v.w ) ); }
vec4_t vec4_degrees( vec4_t v ) { return vec4( internal_vecmath_degrees( v.x ), internal_vecmath_degrees( v.y ), internal_vecmath_degrees( v.z ), internal_vecmath_degrees( v.w ) ); } 
float vec4_distancesq( vec4_t a, vec4_t b ) { float x = b.x - a.x; float y = b.y - a.y; float z = b.z - a.z; float w = b.w - a.w; return x * x + y * y + z * z + w * w; }
float vec4_distance( vec4_t a, vec4_t b ) { float x = b.x - a.x; float y = b.y - a.y; float z = b.z - a.z; float w = b.w - a.w; return internal_vecmath_sqrt( x * x + y * y + z * z + w * w ); }
float vec4_dot( vec4_t a, vec4_t b ) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;  }
vec4_t vec4_exp( vec4_t v ) { return vec4( internal_vecmath_exp( v.x ), internal_vecmath_exp( v.y ), internal_vecmath_exp( v.z ), internal_vecmath_exp( v.w ) ); }
vec4_t vec4_exp2( vec4_t v ) { return vec4( internal_vecmath_exp2( v.x ), internal_vecmath_exp2( v.y ), internal_vecmath_exp2( v.z ), internal_vecmath_exp2( v.w ) ); }
vec4_t vec4_floor( vec4_t v ) { return vec4( internal_vecmath_floor( v.x ), internal_vecmath_floor( v.y ), internal_vecmath_floor( v.z ), internal_vecmath_floor( v.w ) ); }
vec4_t vec4_fmod( vec4_t a, vec4_t b ) { return vec4( internal_vecmath_fmod( a.x, b.x ), internal_vecmath_fmod( a.y, b.y ), internal_vecmath_fmod( a.z, b.z ), internal_vecmath_fmod( a.w, b.w ) ); }
vec4_t vec4_frac( vec4_t v ) { return vec4( internal_vecmath_frac( v.x ), internal_vecmath_frac( v.y ), internal_vecmath_frac( v.z ), internal_vecmath_frac( v.w ) ); }
float vec4_lengthsq( vec4_t v ) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }
float vec4_length( vec4_t v ) { return internal_vecmath_sqrt( v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w ); }
vec4_t vec4_lerp( vec4_t a, vec4_t b, float s ) { return vec4( internal_vecmath_lerp( a.x, b.x, s ), internal_vecmath_lerp( a.y, b.y, s ), internal_vecmath_lerp( a.z, b.z, s ), internal_vecmath_lerp( a.w, b.w, s ) ); }
vec4_t vec4_log( vec4_t v ) { return vec4( internal_vecmath_log( v.x ), internal_vecmath_log( v.y ), internal_vecmath_log( v.z ), internal_vecmath_log( v.w ) ); }
vec4_t vec4_log2( vec4_t v ) { return vec4( internal_vecmath_log2( v.x ), internal_vecmath_log2( v.y ), internal_vecmath_log2( v.z ), internal_vecmath_log2( v.w ) ); }
vec4_t vec4_log10( vec4_t v ) { return vec4( internal_vecmath_log10( v.x ), internal_vecmath_log10( v.y ), internal_vecmath_log10( v.z ), internal_vecmath_log10( v.w ) ); }
vec4_t vec4_max( vec4_t a, vec4_t b ) { return vec4( internal_vecmath_max( a.x, b.x ), internal_vecmath_max( a.y, b.y ), internal_vecmath_max( a.z, b.z ), internal_vecmath_max( a.w, b.w ) ); }
vec4_t vec4_min( vec4_t a, vec4_t b ) { return vec4( internal_vecmath_min( a.x, b.x ), internal_vecmath_min( a.y, b.y ), internal_vecmath_min( a.z, b.z ), internal_vecmath_min( a.w, b.w ) ); }
vec4_t vec4_normalize( vec4_t v ) { float l = internal_vecmath_sqrt( v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w ); return l == 0.0f ? v : vec4( v.x / l, v.y / l, v.z / l, v.w / l ); }
vec4_t vec4_pow( vec4_t a, vec4_t b ) { return vec4( internal_vecmath_pow( a.x, b.x ), internal_vecmath_pow( a.y, b.y ), internal_vecmath_pow( a.z, b.z ), internal_vecmath_pow( a.w, b.w ) ); }
vec4_t vec4_radians( vec4_t v ) { return vec4( internal_vecmath_radians( v.x ), internal_vecmath_radians( v.y ), internal_vecmath_radians( v.z ), internal_vecmath_radians( v.w ) ); } 
vec4_t vec4_rcp( vec4_t v ) { return vec4( internal_vecmath_rcp( v.x ), internal_vecmath_rcp( v.y ), internal_vecmath_rcp( v.z ), internal_vecmath_rcp( v.w ) ); }
vec4_t vec4_round( vec4_t v ) { return vec4( internal_vecmath_round( v.x ), internal_vecmath_round( v.y ), internal_vecmath_round( v.z ), internal_vecmath_round( v.w ) ); }
vec4_t vec4_rsqrt( vec4_t v ) { return vec4( internal_vecmath_rcp( internal_vecmath_sqrt( v.x ) ), internal_vecmath_rcp( internal_vecmath_sqrt( v.y ) ), internal_vecmath_rcp( internal_vecmath_sqrt( v.z ) ), internal_vecmath_rcp( internal_vecmath_sqrt( v.w ) ) ); }
vec4_t vec4_saturate( vec4_t v ) { return vec4( internal_vecmath_saturate( v.x ), internal_vecmath_saturate( v.y ), internal_vecmath_saturate( v.z ), internal_vecmath_saturate( v.w ) ); }
vec4_t vec4_sign( vec4_t v ) { return vec4( internal_vecmath_sign( v.x ), internal_vecmath_sign( v.y ), internal_vecmath_sign( v.z ), internal_vecmath_sign( v.w ) ); }
vec4_t vec4_sin( vec4_t v ) { return vec4( internal_vecmath_sin( v.x ), internal_vecmath_sin( v.y ), internal_vecmath_sin( v.z ), internal_vecmath_sin( v.w ) ); }
vec4_t vec4_sinh( vec4_t v ) { return vec4( internal_vecmath_sinh( v.x ), internal_vecmath_sinh( v.y ), internal_vecmath_sinh( v.z ), internal_vecmath_sinh( v.w ) ); }
vec4_t vec4_smoothstep( vec4_t internal_vecmath_min, vec4_t internal_vecmath_max, vec4_t v ) { return vec4( internal_vecmath_smoothstep( internal_vecmath_min.x, internal_vecmath_max.x, v.x ), internal_vecmath_smoothstep( internal_vecmath_min.y, internal_vecmath_max.y, v.y ), internal_vecmath_smoothstep( internal_vecmath_min.z, internal_vecmath_max.z, v.z ), internal_vecmath_smoothstep( internal_vecmath_min.w, internal_vecmath_max.w, v.w ) ); }
vec4_t vec4_smootherstep( vec4_t internal_vecmath_min, vec4_t internal_vecmath_max, vec4_t v ) { return vec4( internal_vecmath_smootherstep( internal_vecmath_min.x, internal_vecmath_max.x, v.x ), internal_vecmath_smootherstep( internal_vecmath_min.y, internal_vecmath_max.y, v.y ), internal_vecmath_smootherstep( internal_vecmath_min.z, internal_vecmath_max.z, v.z ), internal_vecmath_smootherstep( internal_vecmath_min.w, internal_vecmath_max.w, v.w ) ); }
vec4_t vec4_sqrt( vec4_t v ) { return vec4( internal_vecmath_sqrt( v.x ), internal_vecmath_sqrt( v.y ), internal_vecmath_sqrt( v.z ), internal_vecmath_sqrt( v.w ) ); }
vec4_t vec4_step( vec4_t a, vec4_t b ) { return vec4( internal_vecmath_step( a.x, b.x ), internal_vecmath_step( a.y, b.y ), internal_vecmath_step( a.z, b.z ), internal_vecmath_step( a.w, b.w ) ); }
vec4_t vec4_tan( vec4_t v ) { return vec4( internal_vecmath_tan( v.x ), internal_vecmath_tan( v.y ), internal_vecmath_tan( v.z ), internal_vecmath_tan( v.w ) ); }
vec4_t vec4_tanh( vec4_t v ) { return vec4( internal_vecmath_tanh( v.x ), internal_vecmath_tanh( v.y ), internal_vecmath_tanh( v.z ), internal_vecmath_tanh( v.w ) ); }
vec4_t vec4_trunc( vec4_t v ) { return vec4( internal_vecmath_trunc( v.x ), internal_vecmath_trunc( v.y ), internal_vecmath_trunc( v.z ), internal_vecmath_trunc( v.w ) ); }


// mat44
mat44_t mat44( vec4_t x, vec4_t y, vec4_t z, vec4_t w ) { mat44_t mat; mat.x = x; mat.y = y; mat.z = z; mat.w = w; return mat; }
mat44_t mat44f( float v ) { mat44_t mat; mat.x = vec4f( v ); mat.y = vec4f( v ); mat.z = vec4f( v ); mat.w = vec4f( v ); return mat; }
	
vec4_t mat44_get( mat44_t mat, int row ) { return ( (vec4_t*) &mat )[ row ]; }
void mat44_set( mat44_t mat, int row, vec4_t value ) { ( (vec4_t*) &mat )[ row ] = value; }

// operators
mat44_t mat44_neg( mat44_t m ) { return mat44( vec4_neg( m.x ), vec4_neg( m.y ), vec4_neg( m.z ), vec4_neg( m.w ) ); }

mat44_t mat44_identity( void ) { return mat44( vec4( 1.0f, 0.0f, 0.0f, 0.0f ), vec4( 0.0f, 1.0f, 0.0f, 0.0f ), vec4( 0.0f, 0.0f, 1.0f, 0.0f ), vec4( 0.0f, 0.0f, 0.0f, 1.0f ) ); }

// functions
mat44_t mat44_abs( mat44_t m ) { return mat44( vec4_abs( m.x ), vec4_abs( m.y ), vec4_abs( m.z ), vec4_abs( m.w ) ); }
mat44_t mat44_acos( mat44_t m ) { return mat44( vec4_acos( m.x ), vec4_acos( m.y ), vec4_acos( m.z ), vec4_acos( m.w ) ); }
int mat44_all( mat44_t m ) { return vec4_all( m.x ) && vec4_all( m.y ) && vec4_all( m.z ) && vec4_all( m.w ); }
int mat44_any( mat44_t m ) { return vec4_any( m.x ) || vec4_any( m.y ) || vec4_any( m.z ) || vec4_any( m.w ); }
mat44_t mat44_asin( mat44_t m ) { return mat44( vec4_asin( m.x ), vec4_asin( m.y ), vec4_asin( m.z ), vec4_asin( m.w ) ); }
mat44_t mat44_atan( mat44_t m ) { return mat44( vec4_atan( m.x ), vec4_atan( m.y ), vec4_atan( m.z ), vec4_atan( m.w ) ); }
mat44_t mat44_atan2( mat44_t y, mat44_t x ) { return mat44( vec4_atan2( y.x, x.x ), vec4_atan2( y.y, x.y ), vec4_atan2( y.z, x.z ), vec4_atan2( y.w, x.w ) ); }
mat44_t mat44_ceil( mat44_t m ) { return mat44( vec4_ceil( m.x ), vec4_ceil( m.y ), vec4_ceil( m.z ), vec4_ceil( m.w ) ); }
mat44_t mat44_clamp( mat44_t m, mat44_t internal_vecmath_min, mat44_t internal_vecmath_max ) { return mat44( vec4_clamp( m.x, internal_vecmath_min.x, internal_vecmath_max.x ), vec4_clamp( m.y, internal_vecmath_min.y, internal_vecmath_max.y ), vec4_clamp( m.z, internal_vecmath_min.z, internal_vecmath_max.z ), vec4_clamp( m.w, internal_vecmath_min.w, internal_vecmath_max.w ) ); }
mat44_t mat44_cos( mat44_t m ) { return mat44( vec4_cos( m.x ), vec4_cos( m.y ), vec4_cos( m.z ), vec4_cos( m.w ) ); }
mat44_t mat44_cosh( mat44_t m ) { return mat44( vec4_cosh( m.x ), vec4_cosh( m.y ), vec4_cosh( m.z ), vec4_cosh( m.w ) ); }
mat44_t mat44_degrees( mat44_t m ) { return mat44( vec4_degrees( m.x ), vec4_degrees( m.y ), vec4_degrees( m.z ), vec4_degrees( m.w ) ); } 
mat44_t mat44_exp( mat44_t m ) { return mat44( vec4_exp( m.x ), vec4_exp( m.y ), vec4_exp( m.z ), vec4_exp( m.w ) ); }
mat44_t mat44_exp2( mat44_t m ) { return mat44( vec4_exp2( m.x ), vec4_exp2( m.y ), vec4_exp2( m.z ), vec4_exp2( m.w ) ); }
mat44_t mat44_floor( mat44_t m ) { return mat44( vec4_floor( m.x ), vec4_floor( m.y ), vec4_floor( m.z ), vec4_floor( m.w ) ); }
mat44_t mat44_fmod( mat44_t a, mat44_t b ) { return mat44( vec4_fmod( a.x, b.x ), vec4_fmod( a.y, b.y ), vec4_fmod( a.z, b.z ), vec4_fmod( a.w, b.w ) ); }
mat44_t mat44_frac( mat44_t m ) { return mat44( vec4_frac( m.x ), vec4_frac( m.y ), vec4_frac( m.z ), vec4_frac( m.w ) ); }
mat44_t mat44_lerp( mat44_t a, mat44_t b, float s ) { return mat44( vec4_lerp( a.x, b.x, s ), vec4_lerp( a.y, b.y, s ), vec4_lerp( a.z, b.z, s ), vec4_lerp( a.w, b.w, s ) ); }
mat44_t mat44_log( mat44_t m ) { return mat44( vec4_log( m.x ), vec4_log( m.y ), vec4_log( m.z ), vec4_log( m.w ) ); }
mat44_t mat44_log2( mat44_t m ) { return mat44( vec4_log2( m.x ), vec4_log2( m.y ), vec4_log2( m.z ), vec4_log2( m.w ) ); }
mat44_t mat44_log10( mat44_t m ) { return mat44( vec4_log10( m.x ), vec4_log10( m.y ), vec4_log10( m.z ), vec4_log10( m.w ) ); }
mat44_t mat44_max( mat44_t a, mat44_t b ) { return mat44( vec4_max( a.x, b.x ), vec4_max( a.y, b.y ), vec4_max( a.z, b.z ), vec4_max( a.w, b.w ) ); }
mat44_t mat44_min( mat44_t a, mat44_t b ) { return mat44( vec4_min( a.x, b.x ), vec4_min( a.y, b.y ), vec4_min( a.z, b.z ), vec4_min( a.w, b.w ) ); }
mat44_t mat44_pow( mat44_t a, mat44_t b ) { return mat44( vec4_pow( a.x, b.x ), vec4_pow( a.y, b.y ), vec4_pow( a.z, b.z ), vec4_pow( a.w, b.w ) ); }
mat44_t mat44_radians( mat44_t m ) { return mat44( vec4_radians( m.x ), vec4_radians( m.y ), vec4_radians( m.z ), vec4_radians( m.w ) ); } 
mat44_t mat44_rcp( mat44_t m ) { return mat44( vec4_rcp( m.x ), vec4_rcp( m.y ), vec4_rcp( m.z ), vec4_rcp( m.w ) ); }
mat44_t mat44_round( mat44_t m ) { return mat44( vec4_round( m.x ), vec4_round( m.y ), vec4_round( m.z ), vec4_round( m.w ) ); }
mat44_t mat44_rsqrt( mat44_t m ) { return mat44( vec4_rsqrt( m.x ), vec4_rsqrt( m.y ), vec4_rsqrt( m.z ), vec4_rsqrt( m.w ) ); }
mat44_t mat44_saturate( mat44_t m ) { return mat44( vec4_saturate( m.x ), vec4_saturate( m.y ), vec4_saturate( m.z ), vec4_saturate( m.w ) ); }
mat44_t mat44_sign( mat44_t m ) { return mat44( vec4_sign( m.x ), vec4_sign( m.y ), vec4_sign( m.z ), vec4_sign( m.w ) ); }
mat44_t mat44_sin( mat44_t m ) { return mat44( vec4_sin( m.x ), vec4_sin( m.y ), vec4_sin( m.z ), vec4_sin( m.w ) ); }
mat44_t mat44_sinh( mat44_t m ) { return mat44( vec4_sinh( m.x ), vec4_sinh( m.y ), vec4_sinh( m.z ), vec4_sinh( m.w ) ); }
mat44_t mat44_smoothstep( mat44_t internal_vecmath_min, mat44_t internal_vecmath_max, mat44_t m ) { return mat44( vec4_smoothstep( internal_vecmath_min.x, internal_vecmath_max.x, m.x ), vec4_smoothstep( internal_vecmath_min.y, internal_vecmath_max.y, m.y ), vec4_smoothstep( internal_vecmath_min.z, internal_vecmath_max.z, m.z ), vec4_smoothstep( internal_vecmath_min.w, internal_vecmath_max.w, m.w ) ); }
mat44_t mat44_smootherstep( mat44_t internal_vecmath_min, mat44_t internal_vecmath_max, mat44_t m ) { return mat44( vec4_smootherstep( internal_vecmath_min.x, internal_vecmath_max.x, m.x ), vec4_smootherstep( internal_vecmath_min.y, internal_vecmath_max.y, m.y ), vec4_smootherstep( internal_vecmath_min.z, internal_vecmath_max.z, m.z ), vec4_smootherstep( internal_vecmath_min.w, internal_vecmath_max.w, m.w ) ); }
mat44_t mat44_sqrt( mat44_t m ) { return mat44( vec4_sqrt( m.x ), vec4_sqrt( m.y ), vec4_sqrt( m.z ), vec4_sqrt( m.w ) ); }
mat44_t mat44_step( mat44_t a, mat44_t b ) { return mat44( vec4_step( a.x, b.x ), vec4_step( a.y, b.y ), vec4_step( a.z, b.z ), vec4_step( a.w, b.w ) ); }
mat44_t mat44_tan( mat44_t m ) { return mat44( vec4_tan( m.x ), vec4_tan( m.y ), vec4_tan( m.z ), vec4_tan( m.w ) ); }
mat44_t mat44_tanh( mat44_t m ) { return mat44( vec4_tanh( m.x ), vec4_tanh( m.y ), vec4_tanh( m.z ), vec4_tanh( m.w ) ); }
mat44_t mat44_trunc( mat44_t m ) { return mat44( vec4_trunc( m.x ), vec4_trunc( m.y ), vec4_trunc( m.z ), vec4_trunc( m.w ) ); }


// matrix math

mat44_t mat44_transpose( mat44_t m ) { return mat44( vec4( m.x.x, m.y.x, m.z.x, m.w.x ), vec4( m.x.y, m.y.y, m.z.y, m.w.y ), vec4( m.x.z, m.y.z, m.z.z, m.w.z ), vec4( m.x.w, m.y.w, m.z.w, m.w.w ) ); }
float mat44_determinant( mat44_t m) { return m.x.w * m.y.z * m.z.y * m.w.x - m.x.z * m.y.w * m.z.y * m.w.x - m.x.w * m.y.y * m.z.z * m.w.x + m.x.y * m.y.w * m.z.z * m.w.x + m.x.z * m.y.y * m.z.w * m.w.x - m.x.y * m.y.z * m.z.w * m.w.x - m.x.w * m.y.z * m.z.x * m.w.y + m.x.z * m.y.w * m.z.x * m.w.y + m.x.w * m.y.x * m.z.z * m.w.y - m.x.x * m.y.w * m.z.z * m.w.y - m.x.z * m.y.x * m.z.w * m.w.y + m.x.x * m.y.z * m.z.w * m.w.y + m.x.w * m.y.y * m.z.x * m.w.z - m.x.y * m.y.w * m.z.x * m.w.z - m.x.w * m.y.x * m.z.y * m.w.z + m.x.x * m.y.w * m.z.y * m.w.z + m.x.y * m.y.x * m.z.w * m.w.z - m.x.x * m.y.y * m.z.w * m.w.z - m.x.z * m.y.y * m.z.x * m.w.w + m.x.y * m.y.z * m.z.x * m.w.w + m.x.z * m.y.x * m.z.y * m.w.w - m.x.x * m.y.z * m.z.y * m.w.w - m.x.y * m.y.x * m.z.z * m.w.w + m.x.x * m.y.y * m.z.z * m.w.w; }

int mat44_is_identity( mat44_t m ) { return m.x.x == 1.0f && m.x.y == 0.0f && m.x.z == 0.0f && m.x.w == 0.0f && m.y.x == 0.0f && m.y.y == 1.0f && m.y.z == 0.0f && m.y.w == 0.0f && m.z.x == 0.0f && m.z.y == 0.0f && m.z.z == 1.0f && m.z.w == 0.0f && m.w.x == 0.0f && m.w.y == 0.0f && m.w.z == 0.0f && m.w.w == 1.0f; }

int mat44_inverse( mat44_t* out_matrix, float* out_determinant, mat44_t m ) { float d = mat44_determinant( m ); if( out_determinant ) *out_determinant = d; if( d != 0.0f && out_matrix ) *out_matrix = mat44( vec4( ( m.y.z * m.z.w * m.w.y - m.y.w * m.z.z * m.w.y + m.y.w * m.z.y * m.w.z - m.y.y * m.z.w * m.w.z - m.y.z * m.z.y * m.w.w + m.y.y * m.z.z * m.w.w ) / d, ( m.x.w * m.z.z * m.w.y - m.x.z * m.z.w * m.w.y - m.x.w * m.z.y * m.w.z + m.x.y * m.z.w * m.w.z + m.x.z * m.z.y * m.w.w - m.x.y * m.z.z * m.w.w ) / d, ( m.x.z * m.y.w * m.w.y - m.x.w * m.y.z * m.w.y + m.x.w * m.y.y * m.w.z - m.x.y * m.y.w * m.w.z - m.x.z * m.y.y * m.w.w + m.x.y * m.y.z * m.w.w ) / d, ( m.x.w * m.y.z * m.z.y - m.x.z * m.y.w * m.z.y - m.x.w * m.y.y * m.z.z + m.x.y * m.y.w * m.z.z + m.x.z * m.y.y * m.z.w - m.x.y * m.y.z * m.z.w ) / d ), vec4( ( m.y.w * m.z.z * m.w.x - m.y.z * m.z.w * m.w.x - m.y.w * m.z.x * m.w.z + m.y.x * m.z.w * m.w.z + m.y.z * m.z.x * m.w.w - m.y.x * m.z.z * m.w.w ) / d, ( m.x.z * m.z.w * m.w.x - m.x.w * m.z.z * m.w.x + m.x.w * m.z.x * m.w.z - m.x.x * m.z.w * m.w.z - m.x.z * m.z.x * m.w.w + m.x.x * m.z.z * m.w.w ) / d, ( m.x.w * m.y.z * m.w.x - m.x.z * m.y.w * m.w.x - m.x.w * m.y.x * m.w.z + m.x.x * m.y.w * m.w.z + m.x.z * m.y.x * m.w.w - m.x.x * m.y.z * m.w.w ) / d, ( m.x.z * m.y.w * m.z.x - m.x.w * m.y.z * m.z.x + m.x.w * m.y.x * m.z.z - m.x.x * m.y.w * m.z.z - m.x.z * m.y.x * m.z.w + m.x.x * m.y.z * m.z.w ) / d ), vec4( ( m.y.y * m.z.w * m.w.x - m.y.w * m.z.y * m.w.x + m.y.w * m.z.x * m.w.y - m.y.x * m.z.w * m.w.y - m.y.y * m.z.x * m.w.w + m.y.x * m.z.y * m.w.w ) / d, ( m.x.w * m.z.y * m.w.x - m.x.y * m.z.w * m.w.x - m.x.w * m.z.x * m.w.y + m.x.x * m.z.w * m.w.y + m.x.y * m.z.x * m.w.w - m.x.x * m.z.y * m.w.w ) / d, ( m.x.y * m.y.w * m.w.x - m.x.w * m.y.y * m.w.x + m.x.w * m.y.x * m.w.y - m.x.x * m.y.w * m.w.y - m.x.y * m.y.x * m.w.w + m.x.x * m.y.y * m.w.w ) / d, ( m.x.w * m.y.y * m.z.x - m.x.y * m.y.w * m.z.x - m.x.w * m.y.x * m.z.y + m.x.x * m.y.w * m.z.y + m.x.y * m.y.x * m.z.w - m.x.x * m.y.y * m.z.w ) / d ), vec4( ( m.y.z * m.z.y * m.w.x - m.y.y * m.z.z * m.w.x - m.y.z * m.z.x * m.w.y + m.y.x * m.z.z * m.w.y + m.y.y * m.z.x * m.w.z - m.y.x * m.z.y * m.w.z ) / d, ( m.x.y * m.z.z * m.w.x - m.x.z * m.z.y * m.w.x + m.x.z * m.z.x * m.w.y - m.x.x * m.z.z * m.w.y - m.x.y * m.z.x * m.w.z + m.x.x * m.z.y * m.w.z ) / d, ( m.x.z * m.y.y * m.w.x - m.x.y * m.y.z * m.w.x - m.x.z * m.y.x * m.w.y + m.x.x * m.y.z * m.w.y + m.x.y * m.y.x * m.w.z - m.x.x * m.y.y * m.w.z ) / d, ( m.x.y * m.y.z * m.z.x - m.x.z * m.y.y * m.z.x + m.x.z * m.y.x * m.z.y - m.x.x * m.y.z * m.z.y - m.x.y * m.y.x * m.z.z + m.x.x * m.y.y * m.z.z ) / d ) ); return d != 0.0f; }		
		 

// matrix multiplications

float vec2_mul_vec2( vec2_t a, vec2_t b ) { return vec2_dot( a, b ); }
float vec3_mul_vec3( vec3_t a, vec3_t b ) { return vec3_dot( a, b ); }
float vec4_mul_vec4( vec4_t a, vec4_t b ) { return vec4_dot( a, b ); }

vec4_t vec4_mul_mat44( vec4_t a, mat44_t b ) { return vec4( a.x * b.x.x + a.y * b.y.x + a.z * b.z.x + a.w * b.w.x, a.x * b.x.y + a.y * b.y.y + a.z * b.z.y + a.w * b.w.y, a.x * b.x.z + a.y * b.y.z + a.z * b.z.z + a.w * b.w.z, a.x * b.x.w + a.y * b.y.w + a.z * b.z.w + a.w * b.w.w ); }

vec4_t mat44_mul_vec4( mat44_t a, vec4_t b ) { return vec4( a.x.x * b.x + a.x.y * b.y + a.x.z * b.z + a.x.w * b.w, a.y.x * b.x + a.y.y * b.y + a.y.z * b.z + a.y.w * b.w, a.z.x * b.x + a.z.y * b.y + a.z.z * b.z + a.z.w * b.w, a.w.x * b.x + a.w.y * b.y + a.w.z * b.z + a.w.w * b.w ); }

mat44_t mat44_mul_mat44( mat44_t a, mat44_t b ) { return mat44( vec4( a.x.x * b.x.x + a.x.y * b.y.x + a.x.z * b.z.x + a.x.w * b.w.x, a.x.x * b.x.y + a.x.y * b.y.y + a.x.z * b.z.y + a.x.w * b.w.y, a.x.x * b.x.z + a.x.y * b.y.z + a.x.z * b.z.z + a.x.w * b.w.z, a.x.x * b.x.w + a.x.y * b.y.w + a.x.z * b.z.w + a.x.w * b.w.w ), vec4( a.y.x * b.x.x + a.y.y * b.y.x + a.y.z * b.z.x + a.y.w * b.w.x, a.y.x * b.x.y + a.y.y * b.y.y + a.y.z * b.z.y + a.y.w * b.w.y, a.y.x * b.x.z + a.y.y * b.y.z + a.y.z * b.z.z + a.y.w * b.w.z, a.y.x * b.x.w + a.y.y * b.y.w + a.y.z * b.z.w + a.y.w * b.w.w ), vec4( a.z.x * b.x.x + a.z.y * b.y.x + a.z.z * b.z.x + a.z.w * b.w.x, a.z.x * b.x.y + a.z.y * b.y.y + a.z.z * b.z.y + a.z.w * b.w.y, a.z.x * b.x.z + a.z.y * b.y.z + a.z.z * b.z.z + a.z.w * b.w.z, a.z.x * b.x.w + a.z.y * b.y.w + a.z.z * b.z.w + a.z.w * b.w.w ), vec4( a.w.x * b.x.x + a.w.y * b.y.x + a.w.z * b.z.x + a.w.w * b.w.x, a.w.x * b.x.y + a.w.y * b.y.y + a.w.z * b.z.y + a.w.w * b.w.y, a.w.x * b.x.z + a.w.y * b.y.z + a.w.z * b.z.z + a.w.w * b.w.z, a.w.x * b.x.w + a.w.y * b.y.w + a.w.z * b.z.w + a.w.w * b.w.w ) ); }


#endif /* vecmath_h */

/*
----------------------
	IMPLEMENTATION
----------------------
*/


#ifdef VECMATH_IMPLEMENTATION
#undef VECMATH_IMPLEMENTATION

// TODO: customizable math funcs
#define _CRT_NONSTDC_NO_DEPRECATE 
#define _CRT_SECURE_NO_WARNINGS
#pragma warning( push )
#pragma warning( disable: 4668 ) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#include <math.h>
#pragma warning( pop )


float internal_vecmath_acosf( float x ) { return (float)internal_vecmath_acos( (double) x ); }
float internal_vecmath_asinf( float x ) { return (float)internal_vecmath_asin( (double) x ); }
float internal_vecmath_atanf( float x ) { return (float)internal_vecmath_atan( (double) x ); }
float internal_vecmath_atan2f( float x, float y ) { return (float)internal_vecmath_atan2( (double) x, (double) y ); }
float internal_vecmath_ceilf( float x ) { return (float)internal_vecmath_ceil( (double) x ); }
float internal_vecmath_cosf( float x ) { return (float)internal_vecmath_cos( (double) x ); }
float internal_vecmath_coshf( float x ) { return (float)internal_vecmath_cosh( (double) x ); }
float internal_vecmath_expf( float x ) { return (float)internal_vecmath_exp( (double) x ); }
float internal_vecmath_fabsf( float x ) { return (float)fabs( (double) x ); }
float internal_vecmath_floorf( float x ) { return (float)internal_vecmath_floor( (double) x ); }
float internal_vecmath_fmodf( float x, float y ) { return (float)internal_vecmath_fmod( (double) x, (double) y ); }
float internal_vecmath_logf( float x ) { return (float)internal_vecmath_log( (double) x ); }
float internal_vecmath_log10f( float x ) { return (float)internal_vecmath_log10( (double) x ); }
float internal_vecmath_modff( float x, float* y ) { double yy; float r = (float)modf( (double) x, &yy ); *y = (float) yy; return r; }
float internal_vecmath_powf( float x, float y ) { return (float)internal_vecmath_pow( (double) x, (double) y ); }
float internal_vecmath_sqrtf( float x ) { return (float)internal_vecmath_sqrt( (double) x ); }
float internal_vecmath_sinf( float x ) { return (float)internal_vecmath_sin( (double) x ); }
float internal_vecmath_sinhf( float x ) { return (float)internal_vecmath_sinh( (double) x ); }
float internal_vecmath_tanf( float x ) { return (float)internal_vecmath_tan( (double) x ); }
float internal_vecmath_tanhf( float x ) { return (float)internal_vecmath_tanh( (double) x ); }

#if !defined( _MSC_VER ) || _MSC_VER >= 1800

	float internal_vecmath_log2f( float x ) { return (float)internal_vecmath_log2( (double) x ); }
	float internal_vecmath_roundf( float x ) { return (float)internal_vecmath_round( (double) x ); }
	float internal_vecmath_truncf( float x ) { return (float)internal_vecmath_trunc( (double) x ); }

#else

	float internal_vecmath_log2f( float x ) 
		{ 
		return (float)( internal_vecmath_log10( (double) x ) / internal_vecmath_log10( 2.0 ) ); 
		}
	
	float internal_vecmath_roundf( float x ) 
		{ 
		double i, r;
		double fraction = modf( (double) x, &i );
		modf( 2.0 * fraction, &r );
		return (float)( i + r );
		}

	float internal_vecmath_truncf( float x ) 
		{ 
		return (float)( x > 0.0f ? ( internal_vecmath_floor( (double) x ) ) : ( internal_vecmath_ceil( (double) x ) ) );
		}
#endif


#endif /* VECMATH_IMPLEMENTATION */

/*
------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2021 Mattias Gustavsson

Permission is hereby granted, free of charge, to internal_vecmath_any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in internal_vecmath_all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

------------------------------------------------------------------------------

ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for internal_vecmath_any purpose, 
commercial or non-commercial, and by internal_vecmath_any means.

In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate internal_vecmath_any and internal_vecmath_all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of internal_vecmath_all present and future rights to 
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
*/
