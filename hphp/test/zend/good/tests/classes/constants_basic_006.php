<?hh
class C
{
    const X = E::A;
    public static $a = darray[K => D::V, E::A => K];
}

eval('class D extends C { const V = \'test\'; }');

class E extends D
{
    const A = "hello";
}

const K = "nasty";

var_dump(C::X, C::$a, D::X, D::$a, E::X, E::$a);
