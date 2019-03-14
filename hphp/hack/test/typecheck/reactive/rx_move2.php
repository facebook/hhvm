<?hh // partial

class C {}

<<__Rx>>
function f() {
    $a = \HH\Rx\mutable(new C());
    // OK
    $b = g(\HH\Rx\move($a));
}

<<__Rx, __MutableReturn>>
function g(<<__OwnedMutable>> C $c): C {
    return $c;
}
