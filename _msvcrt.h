﻿#pragma once

#define _ltlfilelen __FILE__ "(" _CRT_STRINGIZE(__LINE__) ") : "

#ifdef _DEBUG
#error _ltlfilelen "warning: 调试版无法使用VC LTL，请切换到Release然后重新编译！"
#endif

#ifndef _DLL
#error  _ltlfilelen "warning: 由于VC LTL必须在MD编译选项才能使用，请将运行调整为MD！"
#endif

#if defined(NDEBUG)&&defined(_DLL)

#include <crtversion.h>

//关闭常用的pintf系列函数的内联操作，因为老版本没有__stdio_common_vswscanf系列函数
#define _NO_CRT_STDIO_INLINE
#define _Build_By_LTL

//静态整合C++库
#define _STATIC_CPPLIB

#pragma comment(linker, "/nodefaultlib:msvcrt.lib")
#pragma comment(linker, "/nodefaultlib:libcpmt.lib")

//解决某些环境不会添加 XP Mode问题
#ifndef _ATL_XP_TARGETING
#ifdef _USING_V110_SDK71_
#define _ATL_XP_TARGETING
#endif
#endif



#if _VC_CRT_MAJOR_VERSION ==14 && _VC_CRT_MINOR_VERSION==0
#define __ltlversion "140"
#elif _VC_CRT_MAJOR_VERSION ==14 && (_VC_CRT_MINOR_VERSION== 10||_VC_CRT_MINOR_VERSION==11)
#define __ltlversion "141"
#else
#error "暂不支持此版本的CRT库"
#endif

#ifdef _ATL_XP_TARGETING
//XP模式，此模式很多编译器特性采用事件实现，并将XP不支持的一些函数静态导入
#define __ltlversionxp "xp"
#define _ACRTXPIMP extern
#define _ACRTXPINLINE __inline
//XP以及以下系统外部导入
#define _ACRTXPIMPINLINE extern

#ifndef _Allow_LTL_Mode
#pragma detect_mismatch("_LTL_Mode", "XPMode")
#endif

#else //_ATL_XP_TARGETING else
//默认模式，此模式编译器新特性将使用Vista新API实现，性能更佳
#define __ltlversionxp
#define _ACRTXPIMP __declspec(dllimport)
#define _ACRTXPINLINE __declspec(dllimport)
//XP以上系统inline以减少导入数量
#define _ACRTXPIMPINLINE __inline

#ifndef _Allow_LTL_Mode
#pragma detect_mismatch("_LTL_Mode", "VistaMode")
#endif
#endif //_ATL_XP_TARGETING

#ifndef _UCRT_VERISON
#error "vc-ltl 并不支持当前目标平台，请切换目标平台版本至 10240/14393/15063（推荐）。切换目标平台仅仅是切换了Windows SDK/UCRT版本，这并不影响你兼容老版本Windows（包括Windows XP）。"
#elif _UCRT_VERISON == 10240

#ifndef _USING_V110_SDK71_
#pragma message(_ltlfilelen "warning: 10240 ucrt 存在的目的仅用于兼容Windows XP工具集正常编译，而你的程序并未采用XP兼容，强烈建议你迁徙目标平台到Windows 10 16299。")
#endif

#elif _UCRT_VERISON == 10586
#error "warning: 10586 ucrt 已经停止支持，请升级到15063 UCRT或者更高。"
#elif _UCRT_VERISON == 14393
#error "warning: 14393 ucrt 已经停止支持，请升级到15063 UCRT或者更高。"
#elif _UCRT_VERISON == 15063
#pragma message(_ltlfilelen "warning: 15063 ucrt 即将在下个Windows 10 SDK发布时删除，请尽快迁徙目标平台到Windows 10 16299。")
#elif _UCRT_VERISON == 16299
//当前使用16229 UCRT
#else
#pragma message(_ltlfilelen "warning: 无法识别此版本是URCT，强烈建议升级到最新VC-LTL然后继续。")
#endif




#if defined(__NO_LTL_LIB) || defined(__Build_LTL)

#pragma message(_ltlfilelen "info: 进入ltl超越模式，请务必手动引用 _msvcrt.cpp ，否则将编译失败。取消定义 __NO_LTL_LIB 可返回普通模式。")

#else

//__NO_LTL_LIB 宏，可以取消对ltlXXX.lib的引用，取消引用后程序体积可以变得更小，但是必须手动在cpp include _msvcrt.cpp，否则将编译失败
#ifndef _LIB
#pragma comment(lib,"ltl" __ltlversionxp ".lib")
#pragma message(_ltlfilelen "info: 进入ltl普通模式，已准备对" "ltl" __ltlversionxp ".lib" "的引用。定义 __NO_LTL_LIB  可切换到超越模式。")
#endif //!_LIB

#endif //__NO_LTL_LIB || __Build_LTL

#ifndef _LIB
//导入VC编译器新特性
#pragma comment(lib,"vc.lib")
//导入UCRT新特性
#pragma comment(lib,"ucrt.lib")
//导入Windows Vista 动态库 msvcrt.dll
#pragma comment(lib,"msvcrt_base.lib")
//导入C++库
#pragma comment(lib,"ltlcprt" __ltlversionxp ".lib")

#if !defined(_NO__LTL_Initialization)
/*
__LTL_Initialization用于初始化 LTL_Initialization.cpp 全局构造

你可以定义 _NO__LTL_Initialization 来移除 LTL 初始化

但是当你使用iostrean以及_lock_locales相关功能时将导致程序崩溃

*/
#ifdef _M_IX86
#pragma comment(linker,"/include:___LTL_Initialization")
#else
#pragma comment(linker,"/include:__LTL_Initialization")
#endif

#endif


#endif //!_LIB

#endif //NDEBUG&&_DLL
