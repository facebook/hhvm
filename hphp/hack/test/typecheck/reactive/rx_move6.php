<?hh
class C {}

<<__Rx>>
function f(): void {
    $a = HH\Rx\mutable(new C());
    // OK - move as rhs of the assignment
    $b = HH\Rx\move($a);
}
