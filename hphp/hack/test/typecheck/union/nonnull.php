//// file1.php
<?hh

newtype B = int;

newtype C as ?int = int;

type D = ?int;

//// file2.php
<?hh

class A {}

newtype E as ?int = int;

function f(
  bool $cond,
  nonnull $nonnull,
  ?int $nullableInt,
  ?A $nullableA,
  B $b,
  C $c,
  D $d,
  E $e,
): void {

  $x = ($cond ? 1 : $nonnull);
  hh_show($x);

  $x = ($cond ? "" : $nonnull);
  hh_show($x);

  $x = ($cond ? new A() : $nonnull);
  hh_show($x);

  $x = ($cond ? $nullableA : $nonnull);
  hh_show($x);

  $x = ($cond ? null : $nonnull);
  hh_show($x);

  $x = ($cond ? $nullableInt : $nonnull);
  hh_show($x);

  $x = ($cond ? vec[] : $nonnull);
  hh_show($x);

  $x = ($cond ? tuple(1, 2) : $nonnull);
  hh_show($x);

  $x = ($cond ? shape('a' => 2) : $nonnull);
  hh_show($x);

  hh_show($b);
  $x = ($cond ? $b : $nonnull);
  hh_show($x);

  $x = ($cond ? $c : $nonnull);
  hh_show($x);
  expect_nonnull($c); // error

  $x = ($cond ? $d : $nonnull);
  hh_show($x);

  $x = ($cond ? $e : $nonnull);
  hh_show($x);
}

function expect_nonnull(nonnull $x): void {}
