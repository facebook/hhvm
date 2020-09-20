<?hh

function zero() { return 0; }
function foo() { return "0x10"; }
function twelve() { return 12; }

function main() {
  var_dump(zero() + foo());
  var_dump(zero() - foo());
  var_dump(zero() / foo());
  var_dump(zero() * foo());

  var_dump(foo() + zero());
  var_dump(foo() - zero());
  try {
    var_dump(foo() / zero());
  } catch (DivisionByZeroException $e) {}
  var_dump(foo() * zero());

  var_dump(twelve() + foo());
  var_dump(twelve() - foo());
  var_dump(twelve() / foo());
  var_dump(twelve() * foo());

  var_dump(foo() + twelve());
  var_dump(foo() - twelve());
  var_dump(foo() / twelve());
  var_dump(foo() * twelve());
}

function setop_main() {
  $a = varray[zero()];
  $a[0] += foo();
  var_dump($a[0]);
  $a = varray[zero()];
  $a[0] -= foo();
  var_dump($a[0]);
  $a = varray[zero()];
  $a[0] /= foo();
  var_dump($a[0]);
  $a = varray[zero()];
  $a[0] *= foo();
  var_dump($a[0]);

  $a = varray[foo()];
  $a[0] += zero();
  var_dump($a[0]);
  $a = varray[foo()];
  $a[0] -= zero();
  var_dump($a[0]);
  $a = varray[foo()];
  try {
    $a[0] /= zero();
    var_dump($a[0]);
  } catch (DivisionByZeroException $e) {}
  $a = varray[foo()];
  $a[0] *= zero();
  var_dump($a[0]);

  $a = varray[twelve()];
  $a[0] += foo();
  var_dump($a[0]);
  $a = varray[twelve()];
  $a[0] -= foo();
  var_dump($a[0]);
  $a = varray[twelve()];
  $a[0] /= foo();
  var_dump($a[0]);
  $a = varray[twelve()];
  $a[0] *= foo();
  var_dump($a[0]);

  $a = varray[foo()];
  $a[0] += twelve();
  var_dump($a[0]);
  $a = varray[foo()];
  $a[0] -= twelve();
  var_dump($a[0]);
  $a = varray[foo()];
  $a[0] /= twelve();
  var_dump($a[0]);
  $a = varray[foo()];
  $a[0] *= twelve();
  var_dump($a[0]);
}
<<__EntryPoint>> function main_entry(): void {
main();
setop_main();
}
