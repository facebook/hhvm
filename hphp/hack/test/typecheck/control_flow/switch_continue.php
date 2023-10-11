<?hh

class A {}
class B {}

function expect_B(B $x): void {}

function f(int $i, bool $b): void {
  $x = new B();
  while ($b) {
    switch ($i) {
      case 1:
        $x = new A();
        continue;
      default:
        $x = new A();
        continue;
    }
    // continue should just break out of the switch, not continue the loop
    // (because PHP weirdness, see ), so the following should be visible.
    // (Otherwise, if continue continued the loop, this would be dead code
    // and wouldn't show up)
    $x = new B();
  }
  // $x is a B, it cannot be an A
  expect_B($x);
}

// Well actually this test will raise an error that we don't "support" continue
// in switch, which is for the best.
