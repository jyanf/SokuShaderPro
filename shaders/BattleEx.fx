//overrides blend
float4 v4Arg0;
float4 v4Arg1;

sampler2D Sampler0 : register(s0);
sampler2D Sampler1 : register(s1);

technique BlendOverrides
{
    pass Normal
    {
        AlphaBlendEnable = True;
        BlendOp = Add;
        SrcBlend = SrcAlpha;
        DestBlend = InvSrcAlpha;
    }
    pass Add
    {
        AlphaBlendEnable = True;
        BlendOp = Add;
        SrcBlend = SrcAlpha;
        DestBlend = One;
    }
    pass Sub
    {
        AlphaBlendEnable = True;
        BlendOp = RevSubtract;
        SrcBlend = SrcAlpha;
        DestBlend = One;
    }
    pass Mul
    {
        AlphaBlendEnable = True;
        BlendOp = Add;
        SrcBlend = Zero;
        DestBlend = SrcColor;
        
    }
}

float4 PS_RevColor(float4 color : COLOR0, float2 tex : TEXCOORD0) : COLOR0
{
    float4 r0 = tex2D(Sampler0, tex) * color;
    return float4(1.0 - r0.rgb, r0.a);
}

technique Extra
{
    pass SubRev
    {
        AlphaBlendEnable = True;
        BlendOp = RevSubtract;
        SrcBlend = SrcAlpha;
        DestBlend = One;
        PixelShader = compile ps_2_0 PS_RevColor();
    }
    //SEPARATEALPHABLENDENABLE
    //
}