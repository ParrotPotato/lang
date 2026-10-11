n = 42;
s = "hello \"world\"";
t = true;
f = false;

a = 1 + 3 * 4 + -5;
a = 10 - 4 - 3;
a = 8 / 2 * 3;
a = (1 + 2) * 3;

u = -a;
u = +a;
u = !t;
u = - -a;
u = -a * b;
u = -a[0];
u = !done(1);

c = a == b;
c = a != b;
c = a < b;
c = a > b;
c = a <= b;
c = a >= b;
c = a + 1 < b * 2 == true;
c = a < b and b < d or !t;

b = tt.update(5 + 4) + 10;
c = object.attribute[idx + 3] + 31;
m = a.b.c;
m = a[0][1];
m = f();
m = f(1, "two", three);
m = f(g(1), h(2)[3]);
m = a.f(1).g(2);
m = a[0][1](2)(3).z;
m = (a + b)[0];

g = (a, b, c);
g = ();

a[0] = 1;
obj.x = 2;
obj.x[2].y = f(3) + 1;

foo(1);
tt.update(5);

{
    x = 1;
    foo();
    {
        y = 2;
    }
}

if a < 3 {
    b = 2;
}

if a == 1 {
    b = 2;
} else if a == 2 {
    b = 3;
} else {
    b = 4;
}

if v = get(); v != 0 {
    use(v);
}

defer foo();
defer a = 1;
defer {
    close(h);
}

return;
return 1;
return a[0] + f(1);
