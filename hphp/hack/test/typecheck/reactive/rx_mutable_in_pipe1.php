<?hh // partial

class A {
    <<__Rx>>
    public function __construct(int $x) {
    }
    <<__Rx, __Mutable>>
    public function f(): void {
    }
}

<<__Rx>>
function g(): void {
    $a = 1 |> \HH\Rx\mutable(new A($$));
    $a->f();
}
