<?hh  //strict

function a_definition(): int {
  return b_definition();
}

function b_definition(): int {
  return 42;
}

class BB {
  public function __construct(int $i) {}
}

class CC extends BB {
}

class DD extends CC {
}

class EE extends DD {
  public function __construct() {
    parent::__construct(1);
  }
}

class FF {}

function test(): void {
  $bb = new BB(1); // should go to B::__construct
  $cc = new CC(1); // should offer choice B::__construct or C
  $dd = new DD(1); // should offer choice B::__construct or D
  $ee = new EE(); // should go to E::__construct
  $ff = new FF(); // should go to F
}
