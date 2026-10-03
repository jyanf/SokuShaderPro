#pragma once
#include "main.hpp"
constexpr char DORG[]{ "\x1b[0m" };
constexpr char DRED[]{ "\x1b[31m" };
constexpr char DGREEN[]{ "\x1b[32m" };
constexpr char DYELLOW[]{ "\x1b[33m" };
constexpr char DBLUE[]{ "\x1b[34m" };
constexpr char DMAGENTA[]{ "\x1b[35m" };
constexpr char DCYAN[]{ "\x1b[36m" };
constexpr char DWHITE[]{ "\x1b[37m" };
constexpr char DGRAY[]{ "\x1b[90m" };
constexpr char DBOLD[]{ "\x1b[1m" };
namespace {

	inline auto parameterTypeName(D3DXPARAMETER_TYPE type) -> const char* {
        switch (type) {
        case D3DXPT_VOID:        return "VOID";
        case D3DXPT_BOOL:        return "BOOL";
        case D3DXPT_INT:         return "INT";
        case D3DXPT_FLOAT:       return "FLOAT";
        case D3DXPT_STRING:      return "STRING";
        case D3DXPT_TEXTURE:     return "TEXTURE";
        case D3DXPT_TEXTURE1D:   return "TEXTURE1D";
        case D3DXPT_TEXTURE2D:   return "TEXTURE2D";
        case D3DXPT_TEXTURE3D:   return "TEXTURE3D";
        case D3DXPT_TEXTURECUBE: return "TEXTURECUBE";
        case D3DXPT_SAMPLER:     return "SAMPLER";
        case D3DXPT_SAMPLER1D:   return "SAMPLER1D";
        case D3DXPT_SAMPLER2D:   return "SAMPLER2D";
        case D3DXPT_SAMPLER3D:   return "SAMPLER3D";
        case D3DXPT_SAMPLERCUBE: return "SAMPLERCUBE";
        case D3DXPT_PIXELSHADER:    return "PIXELSHADER";
        case D3DXPT_VERTEXSHADER:   return "VERTEXSHADER";
        case D3DXPT_PIXELFRAGMENT:  return "PIXELFRAGMENT";
        case D3DXPT_VERTEXFRAGMENT: return "VERTEXFRAGMENT";
        default:                 return "UNKNOWN";
        }
    }
    inline auto safe(LPCSTR str) -> const char* {
        return str ? str : "<none>";
    };

    inline auto parameterClassName(D3DXPARAMETER_CLASS cls) -> const char* {
        switch (cls)
        {
        case D3DXPC_SCALAR:      return "SCALAR";
        case D3DXPC_VECTOR:      return "VECTOR";
        case D3DXPC_MATRIX_ROWS: return "MATRIX_ROWS";
        case D3DXPC_MATRIX_COLUMNS:return "MATRIX_COLUMNS";
        case D3DXPC_OBJECT:      return "OBJECT";
        case D3DXPC_STRUCT:      return "STRUCT";
        default:                 return "UNKNOWN";
        }
    };

    inline void indent(int depth, bool last=false) {
        for (int i = 0; i < depth; ++i)
            std::printf("%s   ", last ? " " : "©¦");
    }




}
namespace spr {
    inline void fx_debug(Effect* This, HRESULT result) {
        // -------------------------------------------------------------------------
        // Result
        // -------------------------------------------------------------------------
        if (FAILED(result) || !This->effect) {
            std::cout << DRED << DBOLD
                << "[D3DXEffect] Creation FAILED: "
                << DORG
                << DRED 
                << "HRESULT = 0x"
                << std::hex << static_cast<unsigned long>(result) << std::dec
                << DORG << "\n";
            return;
        }

        // -------------------------------------------------------------------------
        // Effect description
        // -------------------------------------------------------------------------

        D3DXEFFECT_DESC effectDesc{};
        if (FAILED(This->effect->GetDesc(&effectDesc))) {
            std::cout
                << DRED
                << "[D3DXEffect] GetDesc() failed"
                << DORG
                << "\n";
            return;
        }

        // -------------------------------------------------------------------------
        // Header
        // -------------------------------------------------------------------------

        std::cout
            << DCYAN
            << "©À©¤©¤ Techniques: "
            << DORG
            << effectDesc.Techniques
            << "\n";

        std::cout
            << DCYAN
            << "©À©¤©¤ Parameters: "
            << DORG
            << effectDesc.Parameters
            << "\n";

        std::cout
            << DCYAN
            << "©À©¤©¤ Functions: "
            << DORG
            << effectDesc.Functions
            << "\n";

        // =========================================================================
        // Parameters
        // =========================================================================

        std::cout
            << DYELLOW
            << "©À©¤©¤ Parameters"
            << DORG
            << "\n";

        for (UINT i = 0; i < effectDesc.Parameters; ++i) {
            D3DXHANDLE handle =
                This->effect->GetParameter(nullptr, i);

            D3DXPARAMETER_DESC desc{};

            if (!handle ||
                FAILED(This->effect->GetParameterDesc(handle, &desc)))
            {
                indent(1);
                std::cout << DRED
                    << "©À©¤©¤ [" << i << "] <invalid parameter>"
                    << DORG << "\n";
                continue;
            }

            indent(1);

            std::cout
                << DCYAN
                << "©À©¤©¤  "
                << DBOLD
                << desc.Name
                << DGRAY
                << " [" << parameterTypeName(desc.Type) << ", " << parameterClassName(desc.Class) << "]"
                << DORG << "\n";

            indent(2);
            std::cout
                << "©À©¤©¤ Semantic: "
                << DGRAY
                << safe(desc.Semantic)
                << DORG
                << "\n";

            indent(2);
            std::cout
                << "©À©¤©¤ Type: " << parameterTypeName(desc.Type) //<< " (" << static_cast<int>(desc.Type) << ")"
                << "\n";

            indent(2);
            std::cout
                << "©À©¤©¤ Class: " << parameterClassName(desc.Class) //<< " (" << static_cast<int>(desc.Class) << ")"
                << "\n";

            indent(2);
            std::cout
                << "©À©¤©¤ Dimensions: " << desc.Rows << " x " << desc.Columns 
                << "\n";

            /*indent(2);
            std::cout
                << "©À©¤©¤ Elements: " << desc.Elements
                << "\n";

            indent(2);
            std::cout
                << "©À©¤©¤ Annotations: " << desc.Annotations
                << "\n";

            indent(2);
            std::cout
                << "©À©¤©¤ StructMembers: " << desc.StructMembers
                << "\n";

            */
            indent(2);
            std::cout
                << "©¸©¤©¤ Bytes: " << desc.Bytes
                << "\n";
        }

        // =========================================================================
        // Techniques / Passes
        // =========================================================================

        std::cout << DYELLOW
            << "©¸©¤©¤ Techniques"
            << DORG
            << "\n";

        for (UINT ti = 0; ti < effectDesc.Techniques; ++ti) {
            D3DXHANDLE technique = This->effect->GetTechnique(ti);

            D3DXTECHNIQUE_DESC techniqueDesc{};

            if (!technique || FAILED(This->effect->GetTechniqueDesc(technique, &techniqueDesc))) {
                indent(1);

                std::cout
                    << DRED
                    << "©À©¤©¤ [" << ti << "] <invalid technique>"
                    << DORG
                    << "\n";

                continue;
            }

            const bool lastTechnique = (ti + 1 == effectDesc.Techniques);

            indent(1);

            std::cout
                << (lastTechnique ? "©¸©¤©¤ " : "©À©¤©¤ ")
                << DBLUE
                << DBOLD
                << "Technique: " << safe(techniqueDesc.Name) 
                << DORG
                << "\n";

            indent(2, lastTechnique);

            std::cout
                << "©À©¤©¤ Passes: " << techniqueDesc.Passes
                << "\n";

            indent(2, lastTechnique);

            std::cout
                << "©À©¤©¤ Annotations: " << techniqueDesc.Annotations
                << "\n";

            // ---------------------------------------------------------------------
            // Passes
            // ---------------------------------------------------------------------

            for (UINT pi = 0; pi < techniqueDesc.Passes; ++pi) {
                D3DXHANDLE pass = This->effect->GetPass(technique, pi);
                D3DXPASS_DESC passDesc{};

                if (!pass || FAILED(This->effect->GetPassDesc(pass, &passDesc))) {
                    indent(2);
                    std::cout
                        << DRED
                        << "©¸©¤©¤ [" << pi << "] <invalid pass>"
                        << DORG << "\n";
                    continue;
                }

                const bool lastPass = (pi + 1 == techniqueDesc.Passes);

                indent(2, lastPass);

                std::cout
                    << (lastPass ? "©¸©¤©¤ " : "©À©¤©¤ ")
                    << DMAGENTA
                    << "Pass: "
                    << safe(passDesc.Name)
                    << DORG
                    << "\n";

              
            }
        }
        std::cout << DGREEN << DBOLD
            << "\n[D3DXEffect] Successfully created\n============================================ \n\n"
            << DORG << std::endl;
    }
}