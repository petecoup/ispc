
export uniform int width() { return programCount; }



struct Foo { float f; };

void f(reference uniform Foo foo[], float a) {
    ++foo[a].f;
}

export void f_fu(uniform float RET[], uniform float aFOO[], uniform float b) {
    float a = aFOO[programIndex];
    float f[40], g[40];
    for (uniform int i = 0; i < 40; ++i) {
        f[i] = a;
        g[i] = b;
    }
    if (a < 2)
        f = g;
    RET[programIndex] = f[a];
}

export void result(uniform float RET[]) { 
    RET[programIndex] = 1+programIndex;
    RET[0] = 5;
}
