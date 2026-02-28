<?hh
class C
{
    const X = E::A;
    public static $a = dict[K => D::V, E::A => K];
}

const K = "nasty";
<<__EntryPoint>>
function entrypoint_constants_basic_006(): void {

  eval('class D extends C { const V = \'test\'; }');

  include(__DIR__.'/constants_basic_006.inc');

  var_dump(C::X, C::$a, D::X, D::$a, E::X, E::$a);
}
