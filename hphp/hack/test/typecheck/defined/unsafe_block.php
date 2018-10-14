<?hh //strict

class A {}
class B {}
class C {}

function test1(bool $b): int {
  if ($b) {
    $x = 0;
  } else {
    // UNSAFE_BLOCK
    // the type of $x should be ignored, but we still record that $x is defined
    // in this branch
    $x = "whatever";
  }
  return $x; // ok
}

function test2(bool $b): void {
  if ($b) {
    $x = new A();
    $y = new A();
  } else {
    if ($b) {
      $x = new B(); // this should not be ignored
      $y = new B();
      // UNSAFE_BLOCK
      // I can do completely unsafe things here
      expect_int($x);
      expect_int(make_string());
      $x = "whatever"; // this should be ignored
    } else {
      $x = new C(); // but this should not be ignored
      $y = new C();
    }
  }
  // $x is A | C
  // but not B because it has been set to whatever in the unsafe part
  hh_show($x);
  // $y is A | B | C
  hh_show($y);
}

function expect_int(int $x): void {}
function make_string(): string {
  return "";
}
