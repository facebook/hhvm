<?hh

class C {}

<<__Rx>>
function f(): void {
    $a = \HH\Rx\mutable(new C());
    // ERROR
    \HH\Rx\move($a);
}
