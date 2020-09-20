<?hh // partial

class C {}

<<__Rx>>
function f() {
    $a = \HH\Rx\mutable(new C());
    // ERROR
    \HH\Rx\move($a);
}
