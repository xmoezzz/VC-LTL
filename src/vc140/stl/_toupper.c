/***
*_toupper.c - convert character to uppercase
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines _Toupper()
*
*******************************************************************************/

#include <ctype.h>
#include <stddef.h>
#include <xlocinfo.h>
#include <locale.h>
//#include <awint.h>
#include <yvals.h>
#include <winapi_thunks.h>

/* remove macro definitions of _toupper() and toupper()
 */
#undef  _toupper
#undef  toupper

/***
*int _Toupper(c) - convert character to uppercase
*
*Purpose:
*       _Toupper() is a version of toupper with a locale argument.
*
*Entry:
*       c - int value of character to be converted
*       const _Ctypevec * = pointer to locale info
*
*Exit:
*       returns int value of uppercase representation of c
*
*Exceptions:
*
*******************************************************************************/

_CRTIMP2_PURE int __CLRCALL_PURE_OR_CDECL _Toupper (
        int c,
        const _Ctypevec *ploc
        )
{
        int size;
        unsigned char inbuffer[3];
        unsigned char outbuffer[3];

        //const wchar_t *locale_name;
		LCID _Locale;
        UINT codepage;

        if (ploc == 0)
        {
            //locale_name = ___lc_locale_name_func()[LC_CTYPE];
			_Locale = ___lc_handle_func()[LC_CTYPE];
            codepage = ___lc_codepage_func();
        }
        else
        {
            //locale_name = ploc->_LocaleName;
			_Locale = __acrt_LocaleNameToLCID(ploc->_LocaleName, 0);
            codepage = ploc->_Page;
        }

        //if (locale_name == NULL)
		if(_Locale==0)
        {
            if ( (c >= 'a') && (c <= 'z') )
                c = c - ('a' - 'A');
            return c;
        }

        /* if checking case of c does not require API call, do it */
        if ((unsigned)c < 256)
        {
            if (ploc == 0)
            {
                if (!islower(c))
                {
                    return c;
                }
            }
            else
            {
                if (!(ploc->_Table[c] & _LOWER))
                {
                    return c;
                }
            }
        }

        /* convert int c to multibyte string */
        if (ploc == 0 ? _cpp_isleadbyte((c >> 8) & 0xff)
                      : (ploc->_Table[(c >> 8) & 0xff] & _LEADBYTE) != 0)
        {
            inbuffer[0] = (c >> 8 & 0xff);
            inbuffer[1] = (unsigned char)c;
            inbuffer[2] = 0;
            size = 2;
        } else {
            inbuffer[0] = (unsigned char)c;
            inbuffer[1] = 0;
            size = 1;
        }

        /* convert wide char to uppercase */
        if (0 == (size = __crtLCMapStringA(NULL, _Locale, LCMAP_UPPERCASE,
            (const char *)inbuffer, size, (char *)outbuffer, 3, codepage, TRUE)))
        {
            return c;
        }

        /* construct integer return value */
        if (size == 1)
            return ((int)outbuffer[0]);
        else
            return ((int)outbuffer[1] | ((int)outbuffer[0] << 8));

}
