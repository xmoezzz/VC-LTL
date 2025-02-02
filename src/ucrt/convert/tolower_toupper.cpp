//
// tolower_toupper.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The narrow-string tolower and toupper functions, which convert the integer
// representation of a character to its lowercase equivalent.  These functions
// are defined as macros in the headers.
//
#include <corecrt_internal.h>
#include <ctype.h>
#include <locale.h>



typedef bool (__cdecl internal_istype_l_type)(int, _locale_t);
typedef int  (__cdecl internal_map_type     )(int, _locale_t);

static bool __cdecl internal_islower_l(int const c, _locale_t const locale) throw()
{
    return _islower_l(c, locale) != 0;
}

static bool __cdecl internal_isupper_l(int const c, _locale_t const locale) throw()
{
    return _isupper_l(c, locale) != 0;
}

static int __cdecl internal_map_lower(int const c, _locale_t const locale) throw()
{
    return locale->locinfo->pclmap[c];
}

static int __cdecl internal_map_upper(int const c, _locale_t const locale) throw()
{
    return locale->locinfo->pcumap[c];
}



template <internal_istype_l_type& IsType, internal_map_type& MapType>
static int __cdecl common_tox_l(int const c, DWORD const map_flag, _locale_t const locale) throw()
{
    _LocaleUpdate locale_update(locale);

    if (static_cast<unsigned>(c) < 256)
    {
        if (IsType(c, locale_update.GetLocaleT()))
            return MapType(c, locale_update.GetLocaleT());

        return c;
    }

    // Convert the character to a multibyte string:
    size_t        in_count    {};
    unsigned char in_buffer[3]{};
    if (locale_update.GetLocaleT()->locinfo->_public._locale_mb_cur_max > 1 && _isleadbyte_l(c >> 8 & 0xff, locale_update.GetLocaleT()))
    {
        in_buffer[0] = c >> 8 & 0xff; // Put the lead byte at start of the string
        in_buffer[1] = static_cast<unsigned char>(c);
        in_buffer[2] = 0;
        in_count     = 2;
    }
    else
    {
        // This is undefined behavior: should probably use the wide form instead
        errno = EILSEQ;
        in_buffer[0] = static_cast<unsigned char>(c);
        in_buffer[1] = 0;
        in_count     = 1;
    }

    // Convert the wide character equivalent to the target case:
    unsigned char out_buffer[3]{};
    int const out_count = __acrt_LCMapStringA(
        locale_update.GetLocaleT(),
        locale_update.GetLocaleT()->locinfo->locale_name[LC_CTYPE],
        map_flag,
        reinterpret_cast<char const*>(in_buffer),
        static_cast<int>(in_count),
        reinterpret_cast<char*>(out_buffer),
        static_cast<int>(_countof(out_buffer)),
        locale_update.GetLocaleT()->locinfo->_public._locale_lc_codepage,
        TRUE);

    if (out_count == 0)
        return c;

    // Form the integer return value:
    if (out_count == 1)
        return static_cast<int>(out_buffer[0]);

    return static_cast<int>(out_buffer[1]) | (static_cast<int>(out_buffer[0]) << 8);
}



extern "C" int __cdecl _tolower_l(int const c, _locale_t const locale)
{
    return common_tox_l<internal_isupper_l, internal_map_lower>(c, LCMAP_LOWERCASE, locale);
}

extern "C" int __cdecl tolower(int const c)
{
    return __acrt_locale_changed()
        ? _tolower_l(c, nullptr)
        : __ascii_tolower(c);
}

extern "C" int (__cdecl _tolower)(int const c)
{
    return c - 'A' + 'a';
}



extern "C" int __cdecl _toupper_l(int const c, _locale_t const locale)
{
    return common_tox_l<internal_islower_l, internal_map_upper>(c, LCMAP_UPPERCASE, locale);
}

extern "C" int __cdecl toupper(int const c)
{
    return __acrt_locale_changed()
        ? _toupper_l(c, nullptr)
        : __ascii_toupper(c);
}

extern "C" int (__cdecl _toupper)(int const c)
{
    return c - 'a' + 'A';
}
