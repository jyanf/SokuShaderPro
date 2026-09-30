//Catched binary at 00861428 of size 1668 bytes!
//Parsing...

pixelshader pshaderEx0Limit =
asm {
    ps_1_1
    def c1, 0.25999999, -1, 0.25999999, 0
    def c2, 0, 0, 0, 1
    tex t0
    dp3 r0, t0, c1
    cnd r0, r0.w, c2, t0
    mul r0, r0, v0

// approximately 4 instruction slots used (1 texture, 3 arithmetic)
};

pixelshader pshaderEx0MaskLimit =
asm {
    ps_1_1
    def c1, 0.25999999, -1, 0.25999999, 0
    def c2, 0, 0, 0, 1
    tex t0
    dp3 r0, t0, c1
    cnd r0, r0.w, c2, t0
    mul r0, r0, v0
    add_sat r0, r0, c0
    mul r0, r0, v0

// approximately 6 instruction slots used (1 texture, 5 arithmetic)
};

pixelshader pshaderEx0DPLimit =
asm {
    ps_1_1
    def c1, 0.25999999, -1, 0.25999999, 0
    def c2, 0, 0, 0, 1
    tex t0
    dp3 r0, t0, c1
    cnd r0, r0.w, c2, t0
    mul r0, r0, v0
    dp3 r1.xyz, r0, c0
  + mov r1.w, r0.w
    mul r0, r1, v0

// approximately 6 instruction slots used (1 texture, 5 arithmetic)
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
                def c1, 0.25999999, -1, 0.25999999, 0
                def c2, 0, 0, 0, 1
                tex t0
                dp3 r0, t0, c1
                cnd r0, r0.w, c2, t0
                mul r0, r0, v0

            // approximately 4 instruction slots used (1 texture, 3 arithmetic)
            };
    }
    pass P1
    {
        //No embedded vertex shader
        pixelshader =
            asm {
                ps_1_1
                def c1, 0.25999999, -1, 0.25999999, 0
                def c2, 0, 0, 0, 1
                tex t0
                dp3 r0, t0, c1
                cnd r0, r0.w, c2, t0
                mul r0, r0, v0
                add_sat r0, r0, c0
                mul r0, r0, v0

            // approximately 6 instruction slots used (1 texture, 5 arithmetic)
            };
    }
    pass P2
    {
        //No embedded vertex shader
        pixelshader =
            asm {
                ps_1_1
                def c1, 0.25999999, -1, 0.25999999, 0
                def c2, 0, 0, 0, 1
                tex t0
                dp3 r0, t0, c1
                cnd r0, r0.w, c2, t0
                mul r0, r0, v0
                dp3 r1.xyz, r0, c0
              + mov r1.w, r0.w
                mul r0, r1, v0

            // approximately 6 instruction slots used (1 texture, 5 arithmetic)
            };
    }
}

