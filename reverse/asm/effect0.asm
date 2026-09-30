//Catched binary at 0085A608 of size 1232 bytes!
//Parsing...

pixelshader pshaderBlend =
asm {
    ps_1_1
    tex t0
    tex t1
    mul r0, c0, t0
    mul r1, c1, t1
    add_d2 r0.xyz, r0, r1
    mul r0, r0, v0

// approximately 6 instruction slots used (2 texture, 4 arithmetic)
};


//listing of all techniques and passes with embedded asm listings

technique SampleTexhnique
{
    pass P0
    {
        //No embedded vertex shader
        pixelshader =
            asm {
                ps_1_1
                tex t0
                tex t1
                mul r0, c0, t0
                mul r1, c1, t1
                add_d2 r0.xyz, r0, r1
                mul r0, r0, v0

            // approximately 6 instruction slots used (2 texture, 4 arithmetic)
            };
    }
}



