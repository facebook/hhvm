<?hh

class HH {
  public function __construct(int $i) {}
}

class II extends HH {
  public function __construct() {
    parent::__construct(1);
  }
}

class LL {}

function hh_from_ii_definition(): HH {
  return new II();
}

function ll_type_definition(): LL {
  return new LL();
}

function prim_type_definition(): int {
  return 42;
}

function test_conditional_type(bool $x): void {
  if ($x) {
    $y = new LL();
  } else {
    $y = new HH(2);
  }
  $y; // returns both LL def and HH def
}

function test_standard_types(): void {
  $hh1 = new HH(1); // testing standard class definition
  $hh2 = hh_from_ii_definition(); // testing casting + function return type
  $prim = 40; // $prim should go nowhere

  $hh1; // jump to HH definition
  $hh2; // jump to HH definition
  $prim; // jump to nowhere
  ll_type_definition(); // testing jumping to function return def
  prim_type_definition(); // testing jumping to function def
}
