
export uniform int width() { return programCount; }



void foo(reference float x) {
    if (x <= 2)
        ++x;
}

export void f_fu(uniform float ret[], uniform float a[], uniform float b) {
    float aa = a[programIndex];
    foo(aa);
    ret[programIndex] = aa;
}

export void result(uniform float r[]) {
    r[programIndex] = 1+programIndex;
    r[0] = 2;
    r[1] = 3;
}
