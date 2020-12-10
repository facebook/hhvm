<?hh

class C {}

<<__Rx>>
function f(): void {
    $a = \HH\Rx\mutable(new C());
    // ERROR
    $b = g($a);
}

<<__Rx, __MutableReturn>>
function g(<<__OwnedMutable>> C $c): C {
    return $c;
}
