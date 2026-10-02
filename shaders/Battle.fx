//
// effect1.fx
//
// Reconstructed from the original D3DX9 Effect.
// Original shaders: ps_1_1 / vs_1_1
//
// Notes:
//   v4Arg0 -> original pixel-shader c0
//   x       -> original vertex-shader c0.x
//   y       -> original vertex-shader c1.x
//   width   -> original vertex-shader c2.x
//   height  -> original vertex-shader c3.x
//
// P4/P5/P6 use a vertex shader to generate TEXCOORD1.
// P0/P1/P2/P3 use the caller's normal sprite vertex path.
//

float4 v4Arg0;
float4 v4Arg1;
//cape texture coordinates
float x;
float y;
float width;
float height;

sampler2D Sampler0 : register(s0);
sampler2D Sampler1 : register(s1);



// ============================================================
// P0 - pshaderMask
//
// Original:
//
//   tex t0
//   add_sat r0, t0, c0
//   mul r0, r0, v0
//
// c0 = v4Arg0
// ============================================================

float4 PS_Flash(float2 texCoord : TEXCOORD0, float4 color : COLOR0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, texCoord);
    return saturate(t0 + v4Arg0) * color;
}


// ============================================================
// P1 - pshaderFlash
//
// Original:
//
//   tex t0
//   lrp r0.xyz, c0.w, c0, t0
//   mov r0.w, t0.w
//   mul r0, r0, v0
//
// c0 = v4Arg0
//
// RGB:
//     lerp(t0.rgb, v4Arg0.rgb, v4Arg0.a)
//
// A:
//     t0.a
// ============================================================

float4 PS_Mask(float2 texCoord : TEXCOORD0, float4 color : COLOR0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, texCoord);
    float4 r0;
    
    r0.rgb = lerp(t0.rgb, v4Arg0.rgb, v4Arg0.a);
    r0.a = t0.a;

    return r0 * color;
}


// ============================================================
// P2 - pshaderDP
//
// Original:
//
//   tex t0
//   dp3 r0.xyz, t0, c0
//   mov r0.w, t0.w
//   mul r0, r0, v0
//
// c0 = v4Arg0
// ============================================================

float4 PS_DP(float2 texCoord : TEXCOORD0, float4 color : COLOR0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, texCoord);
    float4 r0;
    
    r0.rgb = dot(t0.rgb, v4Arg0.rgb); //0.299, 0.587, 0.114
    r0.a = t0.a;

    return r0 * color;
}


// ============================================================
// P3 - pshaderDPFlash (unused)
//
// Original:
//
//   tex t0
//   dp3 r0.xyz, t0, c0
//   lrp r0.xyz, c1.w, c1, r0
//   mov r0.w, t0.w
//   mul r0, r0, v0
//
// c0 = v4Arg0
//
// c1 is another Effect constant whose caller has not yet
// been identified.
// ============================================================

float4 PS_DPMask(float2 texCoord : TEXCOORD0, float4 color : COLOR0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, texCoord);
    float4 r0;

    r0.rgb = dot(t0.rgb, v4Arg0.rgb);//gray
    r0.rgb = lerp(r0.rgb, v4Arg1.rgb, v4Arg1.a);

    r0.a = t0.a;
    return r0 * color;
}


// ============================================================
// Ex0 selection
//
// Original:
//
//   def c1, 0.25999999, -1, 0.25999999, 0
//
//   tex t0
//   tex t1
//   dp3 r0, t0, c1
//   cnd r0, r0.w, t1, t0
//
// This chooses between t1 and t0 according to the result of
// the dot-product test.
//
// c1 here is a shader-local constant and is NOT the Effect
// parameter above.
// ============================================================

float4 Utsuho_Select(float4 t0, float4 t1)
{
    if (0.26 * (t0.r + t0.b) - 0.5 > t0.g)
    { //magenta-like color, marking the cape area
        return t1; //cape tex
    } else return t0; //original okuu tex
}


// ============================================================
// P4 - pshaderEx0
//
// Original:
//
//   tex t0
//   tex t1
//   dp3 r0, t0, c1
//   cnd r0, r0.w, t1, t0
//   mul r0, r0, v0
//
// t0 = TEXCOORD0 / Texture0
// t1 = TEXCOORD1 / Texture1
// ============================================================

float4 PS_Utsuho(float2 tex0 : TEXCOORD0, float2 tex1 : TEXCOORD1, float4 color : COLOR0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, tex0);//okuu
    float4 t1 = tex2D(Sampler1, tex1);//cape
    return Utsuho_Select(t0, t1) * color;
}


// ============================================================
// P5 - pshaderEx0Mask
//
// Original:
//
//   tex t0
//   tex t1
//   dp3 r0, t0, c1
//   cnd r0, r0.w, t1, t0
//   mul r0, r0, v0
//   add_sat r0, r0, c0
//   mul r0, r0, v0
//
// c0 = v4Arg0
// ============================================================

float4 PS_UtsuhoFlash(float2 tex0 : TEXCOORD0, float2 tex1 : TEXCOORD1, float4 color : COLOR0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, tex0);
    float4 t1 = tex2D(Sampler1, tex1);

    float4 r0 = Utsuho_Select(t0, t1) * color;
    r0 = saturate(r0 + v4Arg0);

    return r0 * color;
}


// ============================================================
// P6 - pshaderEx0DP
//
// Original:
//
//   tex t0
//   tex t1
//   dp3 r0, t0, c1
//   cnd r0, r0.w, t1, t0
//   mul r0, r0, v0
//   dp3 r1.xyz, r0, c0
//   mov r1.w, r0.w
//   mul r0, r1, v0
//
// c0 = v4Arg0
// ============================================================

float4 PS_UtsuhoDP(float2 tex0 : TEXCOORD0, float2 tex1 : TEXCOORD1, float4 color : COLOR0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, tex0);
    float4 t1 = tex2D(Sampler1, tex1);

    float4 r0 = Utsuho_Select(t0, t1) * color;
    r0.rgb = dot(r0.rgb, v4Arg0.rgb);
    
    return r0 * color;
}

// ============================================================
// Vertex shader used by P4/P5/P6
//
// Original:
//
//   preshader:
//       mul c0.x, c0.x, 0.00390625
//       mul c1.x, c1.x, 0.00390625
//
//   def c4, 0.00390625, 0, -1, 1
//   def c5, 0.00312500005, -0.00416666688, 1, 0
//
//   mul r0.x, v2.x, c2.x
//   mov r1, c4
//   mad oT1.x, r0.x, r1.x, c0.x
//
//   mul r0.x, v2.y, c3.x
//   mad oT1.y, r0.x, r1.x, c1.x
//
//   mad oPos, v0, c5.xyzz, r1.zwyy
//
//   mov oD0, v1
//   mov oT0.xy, v2
//
// ============================================================

struct VS_OUTPUT
{
    float4 Position : POSITION;
    float4 Color : COLOR0;

    float2 Tex0 : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
};

VS_OUTPUT VS_Utsuho(float4 Position : POSITION, float4 Color : COLOR0, float2 TexCoord : TEXCOORD0)
{
    VS_OUTPUT output;
    output.Tex0 = TexCoord;
    output.Color = Color;
    //manually convert xyzrhw to clip space
    output.Tex1.x = TexCoord.x * width / 256.0 + x / 256.0;
    output.Tex1.y = TexCoord.y * height / 256.0 + y / 256.0;

    output.Position.x = 2*(Position.x / 640.0 - 0.5);
    output.Position.y = -2*(Position.y / 480.0 - 0.5);
    output.Position.z = Position.z;
    output.Position.w = Position.w;

    return output;
}


// ============================================================
// Effect
// ============================================================

technique MainTechnique
{
    // --------------------------------------------------------
    // P0 - Flash (shader type 3)
    // --------------------------------------------------------
    pass P0
    {
        PixelShader = compile ps_2_0 PS_Flash();
    }

    // --------------------------------------------------------
    // P1 - Mask (shader type 2)
    // --------------------------------------------------------
    pass P1
    {
        PixelShader = compile ps_2_0 PS_Mask();
    }

    // --------------------------------------------------------
    // P2 - DP (shader type 1)
    // --------------------------------------------------------
    pass P2
    {
        PixelShader = compile ps_2_0 PS_DP();
    }

    // --------------------------------------------------------
    // P3 - DP + Mask (unused)
    // --------------------------------------------------------
    pass P3
    {
        PixelShader = compile ps_2_0 PS_DPMask();
    }

    // --------------------------------------------------------
    // P4 - Utsuho (with cape)
    // --------------------------------------------------------
    pass P4
    {   
        VertexShader = compile vs_2_0 VS_Utsuho();
        PixelShader = compile ps_2_0 PS_Utsuho();
    }

    // --------------------------------------------------------
    // P5 - Utsuho + Flash (shader type 3)
    // --------------------------------------------------------
    pass P5
    {
        VertexShader = compile vs_2_0 VS_Utsuho();
        PixelShader = compile ps_2_0 PS_UtsuhoFlash();
    }

    // --------------------------------------------------------
    // P6 - Ex0 + DP (shader type 1)
    // --------------------------------------------------------
    pass P6
    {
        VertexShader = compile vs_2_0 VS_Utsuho();
        PixelShader = compile ps_2_0 PS_UtsuhoDP();
    }
}
