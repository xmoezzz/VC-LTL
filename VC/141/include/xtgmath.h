/* xtgmath.h internal header */

 #if defined(__cplusplus)
#pragma once
#ifndef _XTGMATH
#define _XTGMATH
#ifndef RC_INVOKED
#include <cstdlib>
#include <xtr1common>

 #pragma pack(push,_CRT_PACKING)
 #pragma warning(push,_STL_WARNING_LEVEL)
 #pragma warning(disable: _STL_DISABLED_WARNINGS)
 #pragma push_macro("new")
 #undef new

_STD_BEGIN
template<class _Ty1,
	class _Ty2>
	using _Common_float_type_t =
		conditional_t<is_same<_Ty1, long double>::value || is_same<_Ty2, long double>::value, long double,
		conditional_t<is_same<_Ty1, float>::value && is_same<_Ty2, float>::value, float,
		double>>;	// find type for two-argument math function
_STD_END

#define _CRTDEFAULT
#define _CRTSPECIAL	_ACRTIMP

#define _GENERIC_MATH1R(FUN, RET, CRTTYPE) \
extern "C" _Check_return_ CRTTYPE RET __cdecl FUN(_In_ double); \
template<class _Ty, \
	class = _STD enable_if_t< _STD is_integral<_Ty>::value>> inline \
	RET FUN(_Ty _Left) \
	{ \
	return (_CSTD FUN(static_cast<double>(_Left))); \
	}

#define _GENERIC_MATH1(FUN, CRTTYPE) \
	_GENERIC_MATH1R(FUN, double, CRTTYPE)

#define _GENERIC_MATH1X(FUN, ARG2, CRTTYPE) \
extern "C" _Check_return_ CRTTYPE double __cdecl FUN(_In_ double, ARG2); \
template<class _Ty, \
	class = _STD enable_if_t< _STD is_integral<_Ty>::value>> inline \
	double FUN(_Ty _Left, ARG2 _Arg2) \
	{ \
	return (_CSTD FUN(static_cast<double>(_Left), _Arg2)); \
	}

#define _GENERIC_MATH2_CALL(FUN, CRTTYPE, CALL_OPT) \
extern "C" _Check_return_ CRTTYPE double CALL_OPT FUN(_In_ double, _In_ double); \
template<class _Ty1, \
	class _Ty2, \
	class = _STD enable_if_t< _STD is_arithmetic<_Ty1>::value && _STD is_arithmetic<_Ty2>::value>> inline \
	_STD _Common_float_type_t<_Ty1, _Ty2> FUN(_Ty1 _Left, _Ty2 _Right) \
	{ \
	typedef _STD _Common_float_type_t<_Ty1, _Ty2> type; \
	return (_CSTD FUN(static_cast<type>(_Left), static_cast<type>(_Right))); \
	}

#define _GENERIC_MATH2(FUN, CRTTYPE) \
	_GENERIC_MATH2_CALL(FUN, CRTTYPE, __cdecl)

template<class _Ty1,
	class _Ty2,
	class = _STD enable_if_t< _STD is_arithmetic<_Ty1>::value && _STD is_arithmetic<_Ty2>::value>> inline
	_STD _Common_float_type_t<_Ty1, _Ty2> pow(const _Ty1 _Left, const _Ty2 _Right)
	{	// bring mixed types to a common type
	typedef _STD _Common_float_type_t<_Ty1, _Ty2> type;
	return (_CSTD pow(static_cast<type>(_Left), static_cast<type>(_Right)));
	}

//_GENERIC_MATH1(abs, _CRTDEFAULT)	// has integer overloads
_GENERIC_MATH1(acos, _CRTDEFAULT)
_GENERIC_MATH1(asin, _CRTDEFAULT)
_GENERIC_MATH1(atan, _CRTDEFAULT)
_GENERIC_MATH2(atan2, _CRTDEFAULT)
_GENERIC_MATH1(ceil, _CRTSPECIAL)
_GENERIC_MATH1(cos, _CRTDEFAULT)
_GENERIC_MATH1(cosh, _CRTDEFAULT)
_GENERIC_MATH1(exp, _CRTDEFAULT)

_GENERIC_MATH1(fabs, _CRT_JIT_INTRINSIC)

_GENERIC_MATH1(floor, _CRTSPECIAL)
_GENERIC_MATH2(fmod, _CRTDEFAULT)
_GENERIC_MATH1X(frexp, _Out_ int *, _CRTSPECIAL)
_GENERIC_MATH1X(ldexp, _In_ int, _CRTSPECIAL)
_GENERIC_MATH1(log, _CRTDEFAULT)
_GENERIC_MATH1(log10, _CRTDEFAULT)
//_GENERIC_MATH1(modf, _CRTDEFAULT)		// types must match
//_GENERIC_MATH2(pow, _CRTDEFAULT)	// hand crafted
_GENERIC_MATH1(sin, _CRTDEFAULT)
_GENERIC_MATH1(sinh, _CRTDEFAULT)
_GENERIC_MATH1(sqrt, _CRTDEFAULT)
_GENERIC_MATH1(tan, _CRTDEFAULT)
_GENERIC_MATH1(tanh, _CRTDEFAULT)

		// C99 MATH FUNCTIONS

		// TEMPLATE FUNCTION fma

inline float _Fma(float _Left, float _Middle, float _Right)
	{	// call float fma
	return (_CSTD fmaf(_Left, _Middle, _Right));
	}

inline double _Fma(double _Left, double _Middle, double _Right)
	{	// call double fma
	return (_CSTD fma(_Left, _Middle, _Right));
	}

inline long double _Fma(long double _Left, long double _Middle,
	long double _Right)
	{	// call long double fma
	return (_CSTD fmal(_Left, _Middle, _Right));
	}

template<class _Ty1,
	class _Ty2,
	class _Ty3> inline
	_STD _Common_float_type_t<_Ty1, _STD _Common_float_type_t<_Ty2, _Ty3>>
	fma(_Ty1 _Left, _Ty2 _Middle, _Ty3 _Right)
	{	// bring mixed types to a common type
	typedef _STD _Common_float_type_t<_Ty1, _STD _Common_float_type_t<_Ty2, _Ty3>> type;
	return (_Fma((type)_Left, (type)_Middle, (type)_Right));
	}

		// TEMPLATE FUNCTION remquo

inline float _Remquo(float _Left, float _Right, int *_Pquo)
	{	// call float remquo
	return (_CSTD remquof(_Left, _Right, _Pquo));
	}

inline double _Remquo(double _Left, double _Right, int *_Pquo)
	{	// call double remquo
	return (_CSTD remquo(_Left, _Right, _Pquo));
	}

inline long double _Remquo(long double _Left, long double _Right, int *_Pquo)
	{	// call long double remquo
	return (_CSTD remquol(_Left, _Right, _Pquo));
	}

template<class _Ty1,
	class _Ty2> inline
	_STD _Common_float_type_t<_Ty1, _Ty2>
	remquo(_Ty1 _Left, _Ty2 _Right, int *_Pquo)
	{	// bring mixed types to a common type
	typedef _STD _Common_float_type_t<_Ty1, _Ty2> type;
	return (_Remquo((type)_Left, (type)_Right, _Pquo));
	}

_GENERIC_MATH1(acosh, _CRTDEFAULT)
_GENERIC_MATH1(asinh, _CRTDEFAULT)
_GENERIC_MATH1(atanh, _CRTDEFAULT)
_GENERIC_MATH1(cbrt, _CRTSPECIAL)
_GENERIC_MATH2(copysign, _CRTSPECIAL)
_GENERIC_MATH1(erf, _CRTSPECIAL)
_GENERIC_MATH1(erfc, _CRTSPECIAL)
_GENERIC_MATH1(expm1, _CRTSPECIAL)
_GENERIC_MATH1(exp2, _CRTSPECIAL)
_GENERIC_MATH2(fdim, _CRTSPECIAL)
//_GENERIC_MATH3(fma, _CRTSPECIAL)	// hand crafted
_GENERIC_MATH2(fmax, _CRTSPECIAL)
_GENERIC_MATH2(fmin, _CRTSPECIAL)
_GENERIC_MATH2(hypot, _CRTSPECIAL)
_GENERIC_MATH1R(ilogb, int, _CRTSPECIAL)
_GENERIC_MATH1(lgamma, _CRTSPECIAL)
_GENERIC_MATH1R(llrint, long long, _CRTSPECIAL)
_GENERIC_MATH1R(llround, long long, _CRTSPECIAL)
_GENERIC_MATH1(log1p, _CRTSPECIAL)
_GENERIC_MATH1(log2, _CRTSPECIAL)
_GENERIC_MATH1(logb, _CRTSPECIAL)
_GENERIC_MATH1R(lrint, long, _CRTSPECIAL)
_GENERIC_MATH1R(lround, long, _CRTSPECIAL)
_GENERIC_MATH1(nearbyint, _CRTSPECIAL)
_GENERIC_MATH2(nextafter, _CRTSPECIAL)
_GENERIC_MATH1X(nexttoward, _In_ long double, _CRTSPECIAL)
_GENERIC_MATH2(remainder, _CRTSPECIAL)
//_GENERIC_MATH2X(remquo, _CRTSPECIAL)	// hand crafted
_GENERIC_MATH1(rint, _CRTSPECIAL)
_GENERIC_MATH1(round, _CRTSPECIAL)
_GENERIC_MATH1X(scalbln, _In_ long, _CRTSPECIAL)
_GENERIC_MATH1X(scalbn, _In_ int, _CRTSPECIAL)
_GENERIC_MATH1(tgamma, _CRTSPECIAL)
_GENERIC_MATH1(trunc, _CRTSPECIAL)

#undef _CRTDEFAULT
#undef _CRTSPECIAL
#undef _GENERIC_MATH1R
#undef _GENERIC_MATH1
#undef _GENERIC_MATH1X
#undef _GENERIC_MATH2_CALL
#undef _GENERIC_MATH2

 #pragma pop_macro("new")
 #pragma warning(pop)
 #pragma pack(pop)
#endif /* RC_INVOKED */
#endif /* _XTGMATH */
 #endif /* defined(__cplusplus) */

/*
 * Copyright (c) by P.J. Plauger. All rights reserved.
 * Consult your license regarding permissions and restrictions.
V6.50:0009 */
