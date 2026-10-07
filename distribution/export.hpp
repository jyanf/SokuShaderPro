#pragma once
#include "main.hpp"
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