<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

interface J {
  public function bar<reify T>(): void;
}

interface I {
  public function foo(): J;
}

function repro2(I $i): void {
  $j = 1 === 2 ? $i->foo() : null;
  if ($j !== null) {
    $j->bar<int>(); // No error
  }
}
