// ============================================================
// Reconstructed Direct3D 9 Effect
//
// Original pixel shader:
//     ps_1_1
//     tex t0
//     tex t1
//     mul r0, c0, t0
//     mul r1, c1, t1
//     add_d2 r0.xyz, r0, r1
//     mul r0, r0, v0
//
// Note:
//   No embedded vertex shader was present in the recovered
//   Effect listing.
// ============================================================

float4 v4Arg0;
float4 v4Arg1;
texture Texture0;
texture Texture1;
sampler2D Sampler0 = sampler_state
{
    Texture = <Texture0>;
};
sampler2D Sampler1 = sampler_state
{
    Texture = <Texture1>;
};

float4 PS_SpellBgBlend(float4 Color : COLOR0, float2 Tex0 : TEXCOORD0, float2 Tex1 : TEXCOORD1) : COLOR0
{
    float4 r0 = v4Arg0 * tex2D(Sampler0, Tex0); //0.5, 0.5, 1.0, 1.0
    float4 r1 = v4Arg1 * tex2D(Sampler1, Tex1); //0.5, 0.5, 1.0, 1.0
    r0.rgb = (r0.rgb + r1.rgb) * 2.0;
    return r0 * Color;
}

technique MainTechnique
{
    pass P0
    {
        PixelShader = compile ps_2_0 PS_SpellBgBlend();
    }
}