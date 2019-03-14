<?hh // partial

class A {}
enum B: int {
  BLAH = 0;
  BLOH = 4;
}

// TODO actually add hh_show's here as we actually care not to have odd things
// like (A | A) ??
function test(bool $b, $untyped, mixed $mixed, nonnull $nonnull, A $a): void {
  if ($b) {
    $x = new A();
  } else {
    $x = $untyped; // Tany
  }
  hh_show($x);
  expect_A($x);

  if ($b) {
    $x = $nonnull;
  } else {
    $x = new A(); // AKdependent
  }
  hh_show($x);
  expect_nonnull($x);
  expect_A($x); // error

  if ($b) {
    $x = $mixed;
  } else {
    $x = new A();
  }
  hh_show($x);
  expect_mixed($x);
  expect_A($x); // error

  if ($b) {
    $x = new Object();
  } else {
    $x = new A();
  }
  hh_show($x);
  expect_object($x);
  expect_A($x); // error

  if ($b) {
    $x = function(string $s1, string $s2): int {
      return 1;
    };
  } else {
    $x = (string $s1, string $s2): string ==> $s1.$s2;
  }
  hh_show($x);
  expect_fun_arraykey($x);
  expect_fun_int($x); // error

  if ($b) {
    $x = ($s1, $s2) ==> $s1.$s2;
  } else {
    $x = ($s1, $s2) ==> \strlen($s1) + \strlen($s2);
  }
  hh_show($x);
  expect_fun_arraykey($x);
  expect_fun_string_string($x);

  if ($b) {
    $x = $a;
  } else {
    $x = new A();
  }
  hh_show($x);
  expect_A($x);

  $x = ($b ? 1 : "");
  hh_show($x);

  $x = ($b ? 1 : 1.1);
  hh_show($x);

  $x = ($b ? 1.1 : "");
  hh_show($x);

  $x = ($b ? B::BLAH : B::BLOH);
  hh_show($x);

  $x = ($b ? 1 : B::BLAH);
  hh_show($x);

  $x = ($b ? new A() : B::BLAH);
  hh_show($x);
}

function expect_fun_string_string((function(string): string) $x): void {}
function expect_fun_arraykey((function(string, string): arraykey) $x): void {}
function expect_fun_int((function(string, string): int) $x): void {}
function expect_shape_ab(shape('a' => nonnull, 'b' => bool, ...) $x): void {}
function expect_shape_a(shape('a' => nonnull, ...) $x): void {}
function expect_bool(bool $x): void {}
function expect_nonnull(nonnull $x): void {}
function expect_object(Object $x): void {}
function expect_A(A $a): void {}
function expect_mixed(mixed $a): void {}

class Object {}
