float4 v4Arg0;
texture Texture0;

sampler2D Sampler0 = sampler_state
{
    Texture = <Texture0>;
};

float4 Utsuho_Select_Limited(float4 t0)
{
    if (0.26 * (t0.r + t0.b) >= t0.g)
    { //magenta-like color, marking the cape area
        return float4(0.0, 0.0, 0.0, 1.0); //black cape
    } else return t0; //original okuu tex
}

float4 PS_Utsuho_Limited(float4 color : COLOR0, float2 tex0 : TEXCOORD0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, tex0);
    float4 r0 = Utsuho_Select_Limited(t0);
    return r0 * color;
}

float4 PS_UtsuhoFlash_Limited(float4 color : COLOR0, float2 tex0 : TEXCOORD0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, tex0);
    float4 r0 = Utsuho_Select_Limited(t0) * color;

    r0 = saturate(r0 + v4Arg0);
    return r0 * color;
}


float4 PS_UtsuhoDP_Limited(float4 color : COLOR0, float2 tex0 : TEXCOORD0) : COLOR0
{
    float4 t0 = tex2D(Sampler0, tex0);
    float4 r0 = Utsuho_Select_Limited(t0) * color;
    
    r0.rgb = dot(r0.rgb, v4Arg0.rgb);
    return r0 * color;
}


technique MainTechnique
{
    pass P0
    {
        PixelShader = compile ps_2_0 PS_Utsuho_Limited();
    }

    pass P1
    {
        PixelShader = compile ps_2_0 PS_UtsuhoFlash_Limited();
    }

    pass P2
    {
        PixelShader = compile ps_2_0 PS_UtsuhoDP_Limited();
    }
}