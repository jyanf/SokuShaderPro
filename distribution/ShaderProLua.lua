---@diagnostic disable miss-field
local oldShaderPro = ShaderPro
package.cpath = package.cpath .. ";./modules/?/?.dll"
require("ShaderPro")
---@meta ShaderProLua
if not ShaderPro then
    error("ShaderPro loaded but not enabled.")
    return nil
end

---@class ShaderProLib
local SPL = {
    _compiled = {}
}
---
---从数据读取并编译（异步）
---@param name string 要编译和注册的Shader名称
---@param data string 提交文本/二进制数据
function SPL.compileData(name, data)
    SPL._compiled[name] = false
    return ShaderPro.SubmitCompileFromData(name, data)
end
---
---从包内文件读取并编译（异步）
---@param name string 要编译和注册的Shader名称
---@param path string 包内文件路径
function SPL.compileFile(name, path)
    SPL._compiled[name] = false
    local data = readfile(path)
    return SPL.compileData(name, data)
end
---
---从外部文件读取并编译（异步）
---@param name string
---@param filepath any
function SPL.compileFileExternal(name, filepath)
    SPL._compiled[name] = false
    return ShaderPro.SubmitCompileFromFile(name, filepath)
end

---获取Shader对应的有效shaderType
---@param name string 提交的Shader名称
---@param tech? string|integer fx源代码中，目标technique的序号（默认为0）或名称
---@param pass? string|integer fx源代码中，目标pass的序号（默认0）或名称
function SPL.getShaderType(name, tech, pass)
    if SPL._compiled[name] or ShaderPro.CheckCompileSuccess(name) then
        SPL._compiled[name] = true
    else
        return nil
    end
    tech = tech or 0
    if type(tech)=="number" then
        tech = tostring(tech)
    end
    pass = pass or 0
    if type(pass)=="number" then
        pass = tostring(pass)
    end
    local id = ShaderPro.GetShaderIdFromURI(string.format("%s.%s.%s", name, tech, pass))
    return id~=0 and id or nil
end

---【高级控制】
---用于精细自定义渲染
---@class ShaderProLib.Effect
SPL.Effect = {}
---
---【高级控制】临时获取shaderType对应Effect
---@param shaderType integer
---@return ShaderProLib.Effect? @Effect对象
function SPL.setShaderReady(shaderType)
    if shaderType<=3 then return nil end
    return ShaderPro.GetEffectReady(shaderType)
end
---
---【高级控制】打开Effect管道
---@return boolean @成功与否
function SPL.Effect:Begins() end
---
---【高级控制】关闭Effect管道
---@return boolean @成功与否
function SPL.Effect:Ends() end
---
---【高级控制】设置浮点数(float)参数
---@param name string @参数名称
---@param values number|number[] @浮点数或其数组
---@return boolean? @成功与否
function SPL.Effect:SetFloats(name, values) end
---
---【高级控制】设置布尔值(bool)参数
---@param name string @参数名称
---@param values boolean|boolean[] @布尔值或其数组
---@return boolean? @成功与否
function SPL.Effect:SetBools(name, values) end
---
---【高级控制】设置整数(int)参数
---@param name string @参数名称
---@param values integer|integer[] @整数或其数组
---@return boolean? @成功与否
function SPL.Effect:SetInts(name, values) end
---
---【高级控制】设置向量(float4)参数
---@param name string @参数名称
---@param values number[]|number[][] @向量或其数组，要求每个向量平铺为4个number
---@return boolean? @成功与否
function SPL.Effect:SetVectors(name, values) end
---
---【高级控制】设置矩阵(matrix)参数
---@param name string @参数名称
---@param values number[]|number[][] @矩阵元素或其数组，要求每个矩阵平铺为16个number
---@return boolean @成功与否
function SPL.Effect:SetMatrixes(name, values) end
---
---【高级控制】设置纹理(texture)参数
---@param name string @参数名称
---@param texture integer @纹理handle
---@return boolean? @成功与否
function SPL.Effect:SetTexture(name, texture) end
---
---【高级控制】设置字符串(string)参数
---@param name string @参数名称
---@param value string @字符串值
---@return boolean? @成功与否
function SPL.Effect:SetString(name, value) end
---
---【高级控制】通过字节串设置参数数据
---@param name string @参数名称
---@param value string @字节串
---@return boolean? @成功与否
function SPL.Effect:SetRaw(name, value) end
---
---【高级控制】提交Effect参数更改
---@return boolean? @成功与否
function SPL.Effect:Commits() end
---
---【高级控制】从指针建立Effect对象<br>
---受限于IPC，回调只能传递指针，故提供此函数
---@param addr integer
---@return ShaderProLib.Effect?
function SPL.Effect.fromPtr() end


---
---【高级控制】注册Effect回调<br>
---将回调与Effect关联，以便在默认流程的Begin前执行<br>一般用于设置Effect(Shader)全局参数
---@alias fxcb fun(fx:ShaderProLib.Effect, obj:battlelib.ObjectBase, techName:string, passName:string):boolean?
---@param name string @参数名称
---@param callback fxcb @回调包装器
---@return boolean? @具体返回值取决于底层实现
function SPL.setEffectCallback(name, callback)
    ShaderPro.SetEffectCallback(name, memory.createcallback(4, function (state, this, obj, tech, pass)
        return callback(ShaderPro.Effect.fromPtr(this), battle.ObjectBase.fromPtr(obj), tech, pass)
    end))
end

---shader getter proxy


return SPL, oldShaderPro