//overrides blend
float4 v4Arg0;
float4 v4Arg1;

sampler2D Sampler0 : register(s0);
sampler2D Sampler1 : register(s1);



technique BlendOverrides
{
    pass Normal
    {
        //BlendOp = ADD;
        //SrcBlend = SRC_ALPHA;
        //DestBlend = INV_SRC_ALPHA;
    }
    pass Add
    {
        
    }
    pass Sub
    {
        
    }
    pass Mul
    {
        
    }
    
    
}