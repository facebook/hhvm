<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function positional_readonly(readonly int $r, named int $z): void {
  var_dump($r, $z);
}

function reversed_names(readonly int $r, named int $a, named int $z): void {
  var_dump($r, $a, $z);
}

class C {
  public function __construct(readonly int $r, named int $z) {
    var_dump($r, $z);
  }
}

function value(int $n): int {
  echo "eval $n\n";
  return $n;
}

<<__EntryPoint>>
function main(): void {
  positional_readonly(readonly 1, z=2);
  positional_readonly(z=2, readonly 1);
  reversed_names(readonly 1, z=3, a=2);
  new C(readonly 1, z=2);
  new C(z=2, readonly 1);
  positional_readonly(readonly value(1), z=value(2));
}
