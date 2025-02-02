﻿#pragma once

#if defined(NDEBUG)&&defined(_DLL)&&defined(__NO_LTL_LIB)
#include <vcruntime_new.h>
#include <corecrt_terminate.h>
#include <stdlib.h>
#include <corecrt_wstdio.h>
#include <Windows.h>
#include <winnt.h>
#include <intrin.h>  
#include <vcruntime_exception.h>
#include <crtdbg.h>
#include <corecrt_wtime.h>
#include <corecrt_io.h>

#include <stdio.h>
#include <internal_shared.h>
#include <locale.h>

#include <sys/stat.h>

#ifdef __NOTHROW_T_DEFINED

#ifdef __cplusplus
extern "C++"
{
	namespace std
	{
		const nothrow_t nothrow;
	}
}
#endif
#endif

extern "C"
{
	/*void __fastcall _guard_check_icall(void*)
	{
	}*/

	//切换到系统的msvrct后无需初始化onexit_table，msvcrt.dll内部会初始化
	bool __cdecl __scrt_initialize_onexit_tables(int const module_type)
	{
		return true;
	}

	void __std_terminate(void)
	{
		terminate();
	}


	struct __crt_stdio_stream_data :public _iobuf
	{
		CRITICAL_SECTION _lock;
	};


	__declspec(dllimport) extern _iobuf _iob[_IOB_ENTRIES];

	FILE* __cdecl __acrt_iob_func(unsigned in)
	{
		return &_iob[in];
	}

	unsigned long long __cdecl wcstoull(
		_In_z_                   wchar_t const* _String,
		_Out_opt_ _Deref_post_z_ wchar_t**      _EndPtr,
		_In_                     int            _Radix
	)
	{
		return _wcstoui64(_String, _EndPtr, _Radix);
	}

	//系统自带有double，用此将其转换为float
	float __cdecl strtof(
		_In_z_                   char const* _String,
		_Out_opt_ _Deref_post_z_ char**      _EndPtr
	)
	{
#pragma warning(suppress : 4244)
		return strtod(_String, _EndPtr);
	}

	float __cdecl wcstof(
		_In_z_                   wchar_t const* _String,
		_Out_opt_ _Deref_post_z_ wchar_t**      _EndPtr
	)
	{
#pragma warning(suppress : 4244)
		return wcstod(_String, _EndPtr);
	}

	BOOL __cdecl __vcrt_InitializeCriticalSectionEx(
		LPCRITICAL_SECTION const critical_section,
		DWORD              const spin_count,
		DWORD              const flags
	)
	{
#ifdef _ATL_XP_TARGETING
		return InitializeCriticalSectionAndSpinCount(critical_section, spin_count);
#else
		return InitializeCriticalSectionEx(critical_section, spin_count, flags);
#endif
	}
	//int __scrt_debugger_hook_flag = 0;

	//void __cdecl _CRT_DEBUGGER_HOOK(int const reserved)
	//{
	//	UNREFERENCED_PARAMETER(reserved);

	//	// We assign zero to the debugger hook flag so that this function is not
	//	// folded when optimized.  The flag is not otherwise used.
	//	__scrt_debugger_hook_flag = 0;
	//}
//#define _CRT_DEBUGGER_HOOK(a) (0)

	void __cdecl __scrt_fastfail(unsigned const code)
	{
		// First see if __fastfail is available, and invoke it if it is.  This will
		// always be availbale on ARM and is always available on Windows 8 and above.
		if (IsProcessorFeaturePresent(PF_FASTFAIL_AVAILABLE))
			__fastfail(code);

		// If __fastfail is not available, fall back to raising an exception that
		// bypasses all exception handlers (via a call to the unhandled exception
		// filter.

		// Notify the debugger if attached.
		//_CRT_DEBUGGER_HOOK(0);

		CONTEXT context_record = {};

#if defined _M_IX86

		__asm
		{
			mov dword ptr[context_record.Eax], eax
			mov dword ptr[context_record.Ecx], ecx
			mov dword ptr[context_record.Edx], edx
			mov dword ptr[context_record.Ebx], ebx
			mov dword ptr[context_record.Esi], esi
			mov dword ptr[context_record.Edi], edi
			mov word ptr[context_record.SegSs], ss
			mov word ptr[context_record.SegCs], cs
			mov word ptr[context_record.SegDs], ds
			mov word ptr[context_record.SegEs], es
			mov word ptr[context_record.SegFs], fs
			mov word ptr[context_record.SegGs], gs
			pushfd
			pop[context_record.EFlags]
		}

		context_record.ContextFlags = CONTEXT_CONTROL;
		context_record.Eip = reinterpret_cast<ULONG>(_ReturnAddress());
		context_record.Esp = reinterpret_cast<ULONG>(_AddressOfReturnAddress());
		context_record.Ebp = *(reinterpret_cast<ULONG*>(_AddressOfReturnAddress()) - 1);

#elif defined _M_X64

		RtlCaptureContext(&context_record);

		ULONG64 const control_pc = context_record.Rip;

		ULONG64 image_base;
		PRUNTIME_FUNCTION const function_entry = RtlLookupFunctionEntry(control_pc, &image_base, nullptr);

		if (function_entry)
		{
			ULONG64 establisher_frame;
			PVOID   handler_data;
			RtlVirtualUnwind(
				UNW_FLAG_NHANDLER,
				image_base,
				control_pc,
				function_entry,
				&context_record,
				&handler_data,
				&establisher_frame,
				nullptr);
		}

		context_record.Rip = reinterpret_cast<ULONGLONG>(_ReturnAddress());
		context_record.Rsp = reinterpret_cast<ULONGLONG>(_AddressOfReturnAddress()) + 8;

#endif

		EXCEPTION_RECORD exception_record = {};
		exception_record.ExceptionCode = STATUS_FATAL_APP_EXIT;
		exception_record.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
		exception_record.ExceptionAddress = _ReturnAddress();

		bool const was_debugger_present = IsDebuggerPresent() == TRUE;

		// Raise an exception that bypasses all exception handlers:
		EXCEPTION_POINTERS exception_pointers = { &exception_record, &context_record };

		SetUnhandledExceptionFilter(nullptr);
		LONG const result = UnhandledExceptionFilter(&exception_pointers);

		// If no handler was found and no debugger was previously attached, then make
		// sure we notify the debugger.
		//if (result == EXCEPTION_CONTINUE_SEARCH && !was_debugger_present)
		//	_CRT_DEBUGGER_HOOK(0);
	}

	//void __cdecl __std_exception_copy(
	//	__std_exception_data const* const from,
	//	__std_exception_data*       const to
	//)
	//{
	//	_ASSERTE(to->_What == nullptr && to->_DoFree == false);

	//	if (!from->_DoFree || !from->_What)
	//	{
	//		to->_What = from->_What;
	//		to->_DoFree = false;
	//		return;
	//	}

	//	size_t const buffer_count = strlen(from->_What) + 1;

	//	auto buffer=static_cast<char*>(malloc(buffer_count));
	//	if (!buffer)
	//	{
	//		return;
	//	}

	//	strcpy_s(buffer, buffer_count, from->_What);
	//	to->_What = buffer;
	//	to->_DoFree = true;
	//}

	//void __cdecl __std_exception_destroy(
	//	__std_exception_data* const data
	//)
	//{
	//	if (data->_DoFree)
	//	{
	//		free(const_cast<char*>(data->_What));
	//	}

	//	data->_DoFree = false;
	//	data->_What = nullptr;
	//}

	void __cdecl _invalid_parameter(
		_In_opt_z_ wchar_t const*,
		_In_opt_z_ wchar_t const*,
		_In_opt_z_ wchar_t const*,
		_In_       unsigned int,
		_In_       uintptr_t
	);

	void __cdecl _invalid_parameter_noinfo_noreturn(void)
	{
		_invalid_parameter(nullptr, nullptr, nullptr, 0, 0);
		_invoke_watson(nullptr, nullptr, nullptr, 0, 0);
	}

	/*errno_t __CRTDECL wmemcpy_s(
		_Out_writes_to_opt_(_N1, _N) wchar_t*       _S1,
		_In_                         rsize_t        _N1,
		_In_reads_opt_(_N)           wchar_t const* _S2,
		_In_                         rsize_t        _N
	)
	{
		return memcpy_s(_S1, _N1 * sizeof(wchar_t), _S2, _N * sizeof(wchar_t));
	}

	errno_t __CRTDECL wmemmove_s(
			_Out_writes_to_opt_(_N1, _N) wchar_t*       _S1,
			_In_                         rsize_t        _N1,
			_In_reads_opt_(_N)           wchar_t const* _S2,
			_In_                         rsize_t        _N
		)
	{
		return memmove_s(_S1, _N1 * sizeof(wchar_t), _S2, _N * sizeof(wchar_t));
	}*/


	//	int __cdecl __stdio_common_vswprintf(
	//		_In_                                    unsigned __int64 _Options,
	//		_Out_writes_z_(_BufferCount)            wchar_t*         _Buffer,
	//		_In_                                    size_t           _BufferCount,
	//		_In_z_ _Printf_format_string_params_(2) wchar_t const*   _Format,
	//		_In_opt_                                _locale_t        _Locale,
	//		va_list          _ArgList
	//	)
	//	{
	//		return _Buffer == NULL ? _vscwprintf_l(_Format, _Locale, _ArgList) : _vswprintf_l(_Buffer, _BufferCount, _Format, _Locale, _ArgList);
	//	}
	//
	//	int __cdecl __stdio_common_vswprintf_s(
	//		_In_                                    unsigned __int64 _Options,
	//		_Out_writes_z_(_BufferCount)            wchar_t*         _Buffer,
	//		_In_                                    size_t           _BufferCount,
	//		_In_z_ _Printf_format_string_params_(2) wchar_t const*   _Format,
	//		_In_opt_                                _locale_t        _Locale,
	//		va_list          _ArgList
	//	)
	//	{
	//		return _Buffer == NULL ? _vscwprintf_l(_Format, _Locale, _ArgList) : _vswprintf_s_l(_Buffer, _BufferCount, _Format, _Locale, _ArgList);
	//	}

#ifdef _ATL_XP_TARGETING

	double __cdecl _difftime64(
		_In_ __time64_t _Time1,
		_In_ __time64_t _Time2
	)
	{
		if (!(_Time2 >= 0 && _Time1 >= 0))
		{
			errno = EINVAL;
			return 0;
		}
		return static_cast<double>(_Time1 - _Time2);
	}

	__declspec(dllimport) struct tm* __cdecl _localtime64(
		_In_ __time64_t const* _Time
	);

	errno_t __cdecl _localtime64_s(
		_Out_ struct tm*        _Tm,
		_In_  __time64_t const* _Time
	)
	{
#pragma warning(suppress : 4996)
		const struct tm* t = _localtime64(_Time);
		if (!t)
			return errno;

		memcpy(_Tm, t, sizeof(struct tm));

		return 0;
	}


	static __inline __time64_t gettime()
	{
		FILETIME FileTime;

		GetSystemTimeAsFileTime(&FileTime);

		__time64_t tmp = ((__time64_t)FileTime.dwHighDateTime << 32) | FileTime.dwLowDateTime;

		tmp = (tmp - 116444736000000000) / 10000000;

		return tmp;
	}

	__time32_t __cdecl _time32(
		_Out_opt_ __time32_t* _Time
	)
	{
		__time64_t tmp = gettime();
		if (_Time)
			*_Time = tmp;

		return tmp;
	}

	__time64_t __cdecl _time64(
		_Out_opt_ __time64_t* _Time
	)
	{
		__time64_t tmp = gettime();
		if (_Time)
			*_Time = tmp;

		return tmp;
	}

	int __cdecl _fseeki64(
		_Inout_ FILE*   _Stream,
		_In_    __int64 _Offset,
		_In_    int     _Origin
	)
	{
		if ((_Stream->_flags & 0x83) == 0 || ( _Origin != SEEK_SET &&_Origin != SEEK_CUR && _Origin != SEEK_END ))
		{
			//无效参数
			errno = EINVAL;
			return -1;
		}

		
		//锁定文件
		_lock_file(_Stream);

		//unset_flags(_IOEOF);
		_Stream->_flags = _Stream->_flags & 0xFFFFFFEF;

		// If seeking relative to the current location, then convert to a seek
		// relative to the beginning of the file.  This accounts for buffering,
		// etc., by letting fseek() tell us where we are:
		if (_Origin == SEEK_CUR)
		{
			_Offset += _ftelli64(_Stream);
			_Origin = SEEK_SET;
		}

		//触发内部缓存刷新机制
		auto result = fseek(_Stream, 0, SEEK_CUR);

		if (result == 0)
			result = _lseeki64(_Stream->_file, _Offset, _Origin) == -1 ? -1 : 0;

		//解锁文件
		_unlock_file(_Stream);

		return result;
	}

#endif

	//总是返回ture，因为老版本没有此函数，不过以前的 _matherr内部他会判断是否存在
	bool __acrt_has_user_matherr()
	{
		return true;
	}

	int __CRTDECL _matherr(_Inout_ struct _exception* _Except);

	int __acrt_invoke_user_matherr(_Inout_ struct _exception* _Except)
	{
		return _matherr(_Except);
	}

	long long __cdecl wcstoll(
		_In_z_                   wchar_t const* _String,
		_Out_opt_ _Deref_post_z_ wchar_t**      _EndPtr,
		_In_                     int            _Radix
	)
	{
		return _wcstoi64(_String, _EndPtr, _Radix);
	}

	long long __cdecl strtoll(
		_In_z_                   char const* _String,
		_Out_opt_ _Deref_post_z_ char**      _EndPtr,
		_In_                     int         _Radix
	)
	{
		return _strtoi64(_String, _EndPtr, _Radix);
	}

	unsigned long long __cdecl strtoull(
		_In_z_                   char const* _String,
		_Out_opt_ _Deref_post_z_ char**      _EndPtr,
		_In_                     int         _Radix
	)
	{
		return _strtoui64(_String, _EndPtr, _Radix);
	}


#ifdef _ATL_XP_TARGETING
#include <Ntsecapi.h>
	errno_t __cdecl rand_s(_Out_ unsigned int* _RandomValue)
	{
		*_RandomValue = 0;

		if (!RtlGenRandom(_RandomValue, sizeof(*_RandomValue)))
		{
			errno = ENOMEM;
			return ENOMEM;
		}
		else
		{
			return 0;
		}
	}
#endif

#ifdef _X86_
#include <math.h>

#pragma function(acos)
	double __cdecl _acos_default(_In_ double _X)
	{
		return acos(_X);
	}
#pragma intrinsic(acos)

#pragma function(asin)
	double __cdecl _asin_default(_In_ double _X)
	{
		return asin(_X);

	}
#pragma intrinsic(asin)

#pragma function(atan)
	double __cdecl _atan_default(_In_ double _X)
	{
		return atan(_X);

	}
#pragma intrinsic(atan)

#pragma function(cos)
	double __cdecl _cos_default(_In_ double _X)
	{
		return cos(_X);

	}
#pragma intrinsic(cos)

	#pragma function(exp)
	double __cdecl _exp_default(_In_ double _X)
	{
		return exp(_X);
	}
#pragma intrinsic(exp)

#pragma function(pow)
	double __cdecl _pow_default(_In_ double _X, _In_ double _Y)
	{
		return pow(_X, _Y);
	}
#pragma intrinsic(pow)

#pragma function(sin)
	double __cdecl _sin_default(_In_ double _X)
	{
		return sin(_X);
	}
#pragma intrinsic(sin)

#pragma function(tan)
	double __cdecl _tan_default(_In_ double _X)
	{
		return tan(_X);
	}
#pragma intrinsic(tan)

#pragma function(log)
	double __cdecl _log_default(_In_ double _X)
	{
		return log(_X);
	}
#pragma intrinsic(log)

#pragma function(log10)
	double __cdecl _log10_default(_In_ double _X)
	{
		return log10(_X);
	}
#pragma intrinsic(log10)

#endif

	__declspec(dllimport) void __cdecl _lock(
		int locknum
	);
	__declspec(dllimport) void __cdecl _unlock(
		int locknum
	);


	static __inline bool IsInternalStream(_iobuf* const stream)
	{
		return ((byte*)stream >= (byte*)_iob) && ((byte*)stream <= (byte*)(_iob + _IOB_ENTRIES -1));
	}

	// Locks a stdio stream.
	void __cdecl _lock_file(FILE* const stream)
	{
		if (IsInternalStream(stream))
			_lock((stream - _iob) + 0x10);
		else
			EnterCriticalSection(&((__crt_stdio_stream_data*)stream)->_lock);
	}



	// Unlocks a stdio stream.
	void __cdecl _unlock_file(FILE* const stream)
	{
		if (IsInternalStream(stream))
			_unlock((stream - _iob) + 0x10);
		else
			LeaveCriticalSection(&((__crt_stdio_stream_data*)stream)->_lock);
	}


	errno_t __cdecl _get_stream_buffer_pointers(
		FILE*   const public_stream,
		char*** const base,
		char*** const ptr,
		int**   const count
	)
	{
		_VALIDATE_RETURN_ERRCODE(public_stream != nullptr, EINVAL);


		if (base)
		{
			*base = &(public_stream->_base);
		}

		if (ptr)
		{
			*ptr = &(public_stream->_ptr);
		}

		if (count)
		{
			*count = &(public_stream->_cnt);
		}

		return 0;
	}


	//msvrct仅支持_Strftime，我们可以将通过字符串转换，得到_Wcsftime
	size_t __cdecl _Wcsftime(
		wchar_t*       const buffer,
		size_t         const max_size,
		wchar_t const* const format,
		tm const*      const timeptr,
		void*          const lc_time_arg
	)
	{
		_VALIDATE_RETURN(buffer != nullptr, EINVAL, 0)
			_VALIDATE_RETURN(max_size != 0, EINVAL, 0)
			*buffer = '\0';

		_VALIDATE_RETURN(format != nullptr, EINVAL, 0)
			_VALIDATE_RETURN(timeptr != nullptr, EINVAL, 0)

			size_t Count = 0;

		unsigned int const lc_time_cp = ___lc_codepage_func();

		auto ch_format = WideCharToMultiByte(lc_time_cp, 0, format, -1, 0,0, nullptr, nullptr);
		if (ch_format==0)
		{
			return 0;
		}

		auto formatA = (char*)malloc(ch_format);
		//开辟2倍缓冲区
		auto BufferA = (char*)malloc(max_size * 2);

		if (formatA==nullptr|| BufferA==nullptr)
		{
			//内存不足
			goto __Error;
		}

		if (WideCharToMultiByte(lc_time_cp, 0,format, -1, formatA, ch_format, nullptr, nullptr))
		{
			//转换失败
			goto __Error;
		}


		Count = _Strftime(BufferA, max_size * 2, formatA, timeptr, lc_time_arg);

		if (Count)
		{
			//
			Count = MultiByteToWideChar(lc_time_cp, 0, BufferA, Count, buffer, max_size);

			//改函数长度并不包含null，因为长度减一
			if (Count)
				--Count;
		}



	__Error:
		if (formatA)
			free(formatA);
		if (BufferA)
			free(BufferA);
		return Count;
	}


	//msvrct仅支持_Getdays，我们可以将通过字符串转换，得到_W_Getdays
	wchar_t* __cdecl _W_Getdays(void)
	{
		auto szDays = _Getdays();
		if (!szDays)
			return nullptr;

		unsigned int const lc_time_cp = ___lc_codepage_func();

		auto ch_Days = MultiByteToWideChar(lc_time_cp, 0, szDays, -1, 0, 0);
		if (ch_Days==0)
		{
			return nullptr;
		}

		//内存申请失败
		auto szDaysW = (wchar_t*)malloc(ch_Days * sizeof(wchar_t));
		if (!szDaysW)
			return nullptr;

		ch_Days = MultiByteToWideChar(lc_time_cp, 0, szDays, -1, szDaysW, ch_Days);

		if (ch_Days)
		{
			return szDaysW;
		}
		else
		{
			free(szDaysW);
			return nullptr;
		}
	}

	//msvrct仅支持_Getmonths，我们可以将通过字符串转换，得到_W_Getmonths
	wchar_t *__cdecl _W_Getmonths(void)
	{
		auto szMonths = _Getmonths();
		if (!szMonths)
			return nullptr;

		unsigned int const lc_time_cp = ___lc_codepage_func();

		auto ch_Months = MultiByteToWideChar(lc_time_cp, 0, szMonths, -1, 0, 0);
		if (ch_Months == 0)
		{
			return nullptr;
		}

		//内存申请失败
		auto szMonthsW = (wchar_t*)malloc(ch_Months * sizeof(wchar_t));
		if (!szMonthsW)
			return nullptr;

		ch_Months = MultiByteToWideChar(lc_time_cp, 0, szMonths, -1, szMonthsW, ch_Months);

		if (ch_Months)
		{
			return szMonthsW;
		}
		else
		{
			free(szMonthsW);
			return nullptr;
		}
	}

	void* __cdecl _W_Gettnames()
	{
		return _Gettnames();
	}

	//
	extern "C++"
	{
		//通过文件句柄获取_stat64
		static __inline int __cdecl _tstat64(
			_In_z_ int     _FileHandle,
			_Out_  struct _stat64* _Stat
		)
		{
			return _fstat64(_FileHandle, _Stat);
		}

		//通过ASCII路径获取_stat64
		static __inline int __cdecl _tstat64(
			_In_z_ char const*     _FileName,
			_Out_  struct _stat64* _Stat
		)
		{
			return _stat64(_FileName, _Stat);
		}

		//通过Unicode路径获取_stat64
		static __inline int __cdecl _tstat64(
			_In_z_ wchar_t const*     _FileName,
			_Out_  struct _stat64* _Stat
		)
		{
			return _wstat64(_FileName, _Stat);
		}


		template<class File, class _statT >
		static __inline int __cdecl common_stat(
			_In_z_ File       _FileName,
			_Out_  _statT* _Stat
		)
		{
			_VALIDATE_CLEAR_OSSERR_RETURN(_Stat != nullptr, EINVAL, -1);

			struct _stat64 _StatTmp;
			auto result = _tstat64(_FileName, &_StatTmp);
			if (result != 0)
			{
				//获取成功，开始转换数据
				_Stat->st_dev = _StatTmp.st_dev;
				_Stat->st_ino = _StatTmp.st_ino;
				_Stat->st_mode = _StatTmp.st_mode;
				_Stat->st_nlink = _StatTmp.st_nlink;
				_Stat->st_uid = _StatTmp.st_uid;
				_Stat->st_gid = _StatTmp.st_gid;
				_Stat->st_rdev = _StatTmp.st_rdev;
				_Stat->st_size = _StatTmp.st_size;
				_Stat->st_atime = _StatTmp.st_atime;
				_Stat->st_mtime = _StatTmp.st_mtime;
				_Stat->st_ctime = _StatTmp.st_ctime;
			}

			return result;
		}
	}

	//_fstat已经改名为_fstat32
#pragma push_macro("_fstat")
#undef _fstat
	__declspec(dllimport) int __cdecl _fstat(
		_In_  int             _FileHandle,
		_Out_ struct _stat32* _Stat
	);

	int __cdecl _fstat32(
		_In_  int             _FileHandle,
		_Out_ struct _stat32* _Stat
	)
	{
		return _fstat(_FileHandle, _Stat);
	}
#pragma pop_macro("_fstat")

	//_fstati64已经改名为_fstat32i64
#pragma push_macro("_fstati64")
#undef _fstati64
	__declspec(dllimport) int __cdecl _fstati64(
		_In_  int                _FileHandle,
		_Out_ struct _stat32i64* _Stat
	);

	int __cdecl _fstat32i64(
		_In_  int                _FileHandle,
		_Out_ struct _stat32i64* _Stat
	)
	{
		return _fstati64(_FileHandle, _Stat);
	}
#pragma pop_macro("_fstati64")


	//msvcrt不支持_fstat64i32，不过我们可以用_fstat64转换
	int __cdecl _fstat64i32(
		_In_  int                _FileHandle,
		_Out_ struct _stat64i32* _Stat
	)
	{
		return common_stat(_FileHandle, _Stat);
	}



	//_stat已经改名为_stat32
#pragma push_macro("_stat")
#undef _stat
	__declspec(dllimport) int __cdecl _stat(
		_In_z_ char const*     _FileName,
		_Out_  struct _stat32* _Stat
	);

	int __cdecl _stat32(
		_In_z_ char const*     _FileName,
		_Out_  struct _stat32* _Stat
	)
	{
		return _stat(_FileName, _Stat);
	}
#pragma pop_macro("_stat")


	//_stati64已经改名为_stat32i64
#pragma push_macro("_stati64")
#undef _stati64
	__declspec(dllimport) int __cdecl _stati64(
		_In_z_ char const*     _FileName,
		_Out_  struct _stat32i64* _Stat
	);

	int __cdecl _stat32i64(
		_In_z_ char const*        _FileName,
		_Out_  struct _stat32i64* _Stat
	)
	{
		return _stati64(_FileName, _Stat);
	}
#pragma pop_macro("_stati64")


	//msvcrt不支持_stat64i32，不过我们可以用_stat64转换
	int __cdecl _stat64i32(
		_In_z_ char const*        _FileName,
		_Out_  struct _stat64i32* _Stat
	)
	{
		return common_stat(_FileName, _Stat);
	}


	//_wstat已经改名为_wstat32，做转发
#pragma push_macro("_wstat")
#undef _wstat
	__declspec(dllimport) int __cdecl _wstat(
		_In_z_ wchar_t const*  _FileName,
		_Out_  struct _stat32* _Stat
	);

	int __cdecl _wstat32(
		_In_z_ wchar_t const*  _FileName,
		_Out_  struct _stat32* _Stat
	)
	{
		return _wstat(_FileName, _Stat);
	}
#pragma pop_macro("_wstat")

	//_wstati64已经改名为_wstat32i64，做转发
#pragma push_macro("_wstati64")
#undef _wstati64
	__declspec(dllimport) int __cdecl _wstati64(
		_In_z_ wchar_t const*     _FileName,
		_Out_  struct _stat32i64* _Stat
	);

	int __cdecl _wstat32i64(
		_In_z_ wchar_t const*     _FileName,
		_Out_  struct _stat32i64* _Stat
	)
	{
		return _wstati64(_FileName, _Stat);
	}
#pragma pop_macro("_wstati64")

	//msvcrt不支持_wstat64i32，不过我们可以用_wstat64转换
	int __cdecl _wstat64i32(
		_In_z_ wchar_t const*     _FileName,
		_Out_  struct _stat64i32* _Stat
	)
	{
		return common_stat(_FileName, _Stat);
	}


	//msvcrt没有_ftelli64，不过好在有fgetpos
	__int64 __cdecl _ftelli64(
		_Inout_ FILE* _Stream
	)
	{
		fpos_t _Position;

		if (fgetpos(_Stream, &_Position)!=0)
		{
			return -1;
		}
		else
		{
			return _Position;
		}
	}

//	typedef struct {
//		long osfhnd;    /* underlying OS file HANDLE */ //CreateFile返回的句柄
//		char osfile;    /* attributes of file (e.g., open in text mode?) */
//		char pipech;    /* one char buffer for handles opened on pipes */
//#ifdef _MT
//		int lockinitflag;
//		CRITICAL_SECTION lock;
//#endif  /* _MT */
//	}   ioinfo;
//
//	__declspec(dllimport) ioinfo* __pioinfo[];
}

#ifdef __cplusplus
extern "C++"
{
	//void* __CRTDECL operator new(
	//	size_t _Size
	//	)
	//{
	//	return malloc(_Size);
	//}

	//void* __CRTDECL operator new[](
	//		size_t _Size
	//		)
	//{
	//	return malloc(_Size);
	//}

	//void __CRTDECL operator delete(
	//		void* _Block
	//		) throw()
	//{
	//	free(_Block);
	//}

	//void __CRTDECL operator delete[](
	//	void* _Block
	//	)
	//{
	//	free(_Block);
	//}

	void __CRTDECL operator delete(
			void*  _Block,
			size_t _Size
			)
	{
		operator delete(_Block);
	}

	void __CRTDECL operator delete[](
		void* _Block,
		size_t _Size
		)
	{
		operator delete[](_Block);
	}
}
#endif // !__cplusplus

#endif //NDEBUG&&_DLL&&__NO_LTL_LIB