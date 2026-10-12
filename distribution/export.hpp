#pragma once
//#include "main.hpp"
#include <Windows.h>

#ifdef SPR_EXPORTS
#define SPR_API extern "C" __declspec(dllexport)
#else
#define SPR_API extern "C" __declspec(dllimport)
#endif

// 提交编译任务：可通过 filepath 或直接传入二进制数据(fx源码或编译后的字节码)。
// 优先使用 filepath，若失败将 fallback 至后者
// name: effect 名称（UTF-8 / narrow string）
// data: 指向二进制数据 和 size: 数据大小（字节）
// filepath: 宽字符路径（Windows 路径）
SPR_API void SubmitCompile(const char* name, const void* data, size_t size, const wchar_t* filepath);

// 通过文件路径提交编译任务（异步）
SPR_API void SubmitCompileFromFile(const char* name, const wchar_t* filepath);

// 通过内存数据提交编译任务（异步）
SPR_API void SubmitCompileFromData(const char* name, const void* data, size_t size);

//检查当前异步编译是否成功
SPR_API bool CheckCompileSuccess(const char* name);

//根据名称，设置默认渲染时，Begin之前的callback，一般用于设置参数
//void(__fastcall*callback)(SokuLib::v2::AnimationObject*, HANDLE, const char*, const char*)
SPR_API void SetEffectCallback(const char* name, void* pf);
SPR_API void UnsetEffectCallback(const char* name);

// 根据 URI 查找 shader id（shaderType）
// 返回 0 表示未找到或错误
SPR_API int GetShaderIdFromURI(const char* uri);

// 根据 shader id 切换并返回已准备好的 Effect HANDLE
// 失败时返回 NULL
SPR_API HANDLE GetEffectReady(int shaderType);

// 根据 GetEffectReady 返回的 Effect HANDLE 执行 Begins/Ends 操作（用于外部灵活控制渲染流程）
// 若返回值为false，代表内部状态出错，此时不应使用
SPR_API bool Effect_Begins(HANDLE effectPtr);
SPR_API bool Effect_Ends(HANDLE effectPtr);

// 按类型和名称设置fx参数
// paraData为指向对应类型变量（数组）的指针
// 指定count>1则为数组版本
// 支持的typeName：
/*TypeName	C/C++					Hlsl
* "float"	—— FLOAT*				—— float, float[]
* "int"		—— INT*					—— int, int[]
* "bool"	—— bool*				—— bool, bool[]
* "vector"	—— D3DXVECTOR4*			—— float4, float4[]
* "matrix"	—— D3DXMATRIX*			—— matrix, matrix[]
* "texture"	—— IDirect3DTexture9*	—— texture
* "string"	—— CHAR*				—— string
* "raw", ""	—— BYTE*				—— any
*/
// 返回值表示成功与否
SPR_API bool Effect_SetValues(HANDLE effectPtr, const char* paraName, const void* paraData, UINT count, const char* typeName);

// 设置所有fx参数后提交修改
// 返回值表示成功与否
SPR_API bool Effect_Commits(HANDLE effectPtr);