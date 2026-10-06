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
    return ShaderPro.SubmitCompileFromFile(name, data)
end

---获取Shader对应的有效shaderType
---@param name string 提交的Shader名称
---@param tech? string|integer fx源代码中，目标technique的序号（默认为0）或名称
---@param pass? string|integer fx源代码中，目标pass的序号（默认0）或名称，
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
---用于自定义渲染流程
---@class ShaderProLib.Shader
---@field handle lightuserdata
SPL.Shader = {
    new = function (handle)
        return setmetatable({
            handle = handle,
        }, SPL.Shader)
    end
}
---
---【高级控制】打开Shader管道
---@return boolean @成功与否
function SPL.Shader:Begins()
    return ShaderPro.Effect_Begins(self.handle)
end
---
---【高级控制】关闭Shader管道
---@return boolean @成功与否
function SPL.Shader:Ends()
    return ShaderPro.Effect_Ends(self.handle)
end
---
---【高级控制】临时获取shaderType对应Shader的句柄
---@param shaderType integer
---@return ShaderProLib.Shader? @Shader句柄
function SPL.setShaderReady(shaderType)
    if shaderType<=3 then return nil end
    local handle = ShaderPro.GetEffectReady(shaderType)
    if not handle then return nil end
    return SPL.Shader.new(handle)
end


return SPL, oldShaderPro