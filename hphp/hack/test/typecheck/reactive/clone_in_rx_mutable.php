<?hh

class A {
    <<__Rx, __Mutable>>
    public function g(): void {
    }
}

<<__Rx>>
function f(A $a): void {
    $b = \HH\Rx\mutable(clone $a);
    $b->g();
}
