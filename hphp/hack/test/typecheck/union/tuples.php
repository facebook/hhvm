<?hh // partial

class A {}

/* We use a few hh_show here instead of expect functions for multiple reasons:
 * - we want the types to _be_ unions, not any subtype of those unions
 * - we actually care not to have odd things like (A | A). Those odd things
 *   might cause type size to explode.
 */
function test(
  bool $b,
  mixed $c,
  (int, A) $t1,
  (int, A) $t2,
  (string, (A, bool)) $t3,
  (float, int, bool) $t4,
): void {
  if ($b) {
    $x = tuple(1, "w", true);
  } else {
    $x = tuple(3, "", false);
  }
  hh_show($x);
  expect_tuple_int_string_bool($x);

  if ($b) {
    $x = tuple(1, 3, true);
  } else {
    $x = tuple($c, "", false);
  }
  hh_show($x);
  expect_tuple_mixed_arraykey_bool($x);

  if ($b) {
    $x = tuple(1, 3, false, true);
  } else {
    $x = tuple($c, "", false);
  }
  hh_show($x);
  expect_tuple_mixed_arraykey_bool($x); // error

  if ($b) {
    $x = $t1;
  } else {
    $x = $t2;
  }
  hh_show($x);
  expect_tuple_int_A($x);

  if ($b) {
    $x = $t1;
  } else {
    $x = $t3;
  }
  hh_show($x);

  if ($b) {
    $x = $t1;
  } else {
    $x = $t4;
  }
  hh_show($x);

  if ($b) {
    $x = $t3;
  } else {
    $x = $t4;
  }
  hh_show($x);

  if ($b) {
    $x = tuple(1, new A());
  } else {
    $x = $t1;
  }
  hh_show($x);
}

function expect_tuple_int_string_bool((int, string, bool) $x): void {}
function expect_tuple_int_A((int, A) $x): void {}
function expect_tuple_mixed_arraykey_bool((mixed, arraykey, bool) $x): void {}
function expect_bool(bool $x): void {}
function expect_nonnull(nonnull $x): void {}
function expect_nullable_bool(?bool $x): void {}
function expect_object(Object $x): void {}
function expect_A(A $a): void {}
function expect_mixed(mixed $a): void {}

class Object {}
