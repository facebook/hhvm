<?hh // partial

class A {
  public function __construct(public string $s) {}
}

<<__Rx>>
function f(string $s): void {
  $s[0] = 'a';
}

<<__Rx>>
function g(<<__Mutable>> A $a): void{
  $a->s[0] = 'a';
}
