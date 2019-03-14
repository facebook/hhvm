<?hh // partial

class C {}

<<__Rx>>
function f() {
    $a = \HH\Rx\mutable(new C());
    // ERROR
    $b = g($a);
}

<<__Rx, __MutableReturn>>
function g(<<__OwnedMutable>> C $c): C {
    return $c;
}
