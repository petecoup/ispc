

export uniform int width() { return programCount; }

struct Foo {
    float f;
    uniform int a;
    float y;
};

extern void aa(reference Foo f);
extern void bb(reference Foo f[]);

typedef float<3> float3;

void set(uniform float3 f[], int offset, float3 val) {
    f[offset] = val;
}

export void f_f(uniform float RET[], uniform float aFOO[]) {
    float a = aFOO[programIndex]; 

    uniform float3 foo[35];
    for (uniform int i = 0; i < 35; ++i) {
        foo[i].x = i;
        foo[i].y = -1;
        foo[i].z = 2*i;
    }

    float3 bar = { programIndex, 2*programIndex, 3*programIndex };
    set(foo, a-1, bar);

    RET[programIndex] = foo[programIndex].z; 
}

export void result(uniform float RET[]) {
    RET[programIndex] = 3*programIndex;
}

