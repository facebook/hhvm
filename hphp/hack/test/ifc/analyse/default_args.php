<?hh

class C {
  <<__InferFlows>>
  public function __construct(public int $i, public int $j = 42) { }
}

<<__InferFlows>>
function constructor_with_default_omitted(int $x): void {
  new C($x);
}

<<__InferFlows>>
function constructor_with_default_supplied(int $x, int $y): void {
  new C($x, $y);
}

<<__InferFlows>>
function f(int $i, int $j = 42): void { }

<<__InferFlows>>
function function_with_default_omitted(int $x): void {
  f($x);
}
