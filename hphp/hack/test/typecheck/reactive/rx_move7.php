<?hh // partial

class C {}

<<__Rx>>
function f() {
    $a = HH\Rx\mutable(new C());
    $b = HH\Rx\mutable(new C());
    // ERROR
    $b = HH\Rx\move($a) + HH\Rx\move($b);
}
