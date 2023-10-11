<?hh

class A {
  const string X = 0;
}

/* We use a few hh_show here instead of expect functions for multiple reasons:
 * - we want the types to _be_ unions, not any subtype of those unions
 * - we actually care not to have odd things like (A | A). Those odd things
 *   might cause type size to explode.
 */
function test1(
  bool $b,
  varray<int> $arrayint,
  varray<int> $arrayint2,
  darray<int, bool> $arrayIntBool,
  darray<int, bool> $arrayIntBool2,
  varray<int> $va1,
  varray<int> $va2,
  varray_or_darray<int> $varraydarray,
  varray_or_darray<int> $varraydarray2,
  darray<int, int> $darray1,
  darray<int, int> $darray2,
  darray<int, string> $darray3,
): void {
  if ($b) {
    $x = varray[]; // AKvarray
  } else {
    $x = varray[]; // AKvarray
  }
  hh_show($x);
  hh_show($x[0]);
  expect_array($x);

  if ($b) {
    $x = varray[]; // AKvarray
  } else {
    $x = varray[1]; // AKvarray
  }
  hh_show($x);
  hh_show($x[0]);
  expect_int_array($x);

  $x = ($b ? varray[1] : varray[2, 3]);
  hh_show($x);
  hh_show($x[0]);

  $x = ($b ? $va1 : $va2);
  hh_show($x);
  hh_show($x[0]);

  $x = ($b ? $va1 : varray[3, 4]);
  hh_show($x);
  hh_show($x[0]);

  if ($b) {
    $x = varray[0]; // AKvarray
  } else {
    $x = varray[""]; // AKvarray
  }
  hh_show($x);
  hh_show($x[0]);
  expect_arraykey_varray($x);
  expect_arraykey_array($x);
  expect_int_array($x); // error
  expect_string_array($x); // error

  $x = ($b ? varray[0] : varray[2, 4]);
  hh_show($x);
  hh_show($x[0]);

  if ($b) {
    $x = $arrayint; // AKvarray
  } else {
    $x = varray[1]; // AKvarray
  }
  hh_show($x);
  hh_show($x[0]);
  expect_int_array($x);

  if ($b) {
    $x = varray[0]; // AKvarray
  } else {
    $x = varray[""]; // AKvarray
  }
  hh_show($x);
  hh_show($x[0]);
  expect_arraykey_varray($x);
  expect_arraykey_array($x);
  expect_int_array($x); // error
  expect_string_array($x); // error

  if ($b) {
    $x = darray[1 => true];
  } else {
    $x = darray[2 => false];
  }
  hh_show($x);
  hh_show($x[0]);

  if ($b) {
    $x = darray[1 => true];
  } else {
    $x = darray[2 => "false"];
  }
  hh_show($x);
  hh_show($x[0]);

  if ($b) {
    $x = darray[1 => 2];
  } else {
    $x = $darray1;
  }
  hh_show($x);
  hh_show($x[0]);

  if ($b) {
    $x = $darray1;
  } else {
    $x = $darray2;
  }
  hh_show($x);
  hh_show($x[0]);

  if ($b) {
    $x = $darray1;
  } else {
    $x = $darray3;
  }
  hh_show($x);
  hh_show($x[0]);

  $x = ($b ? darray[1 => true, 2 => ""] : darray[1 => true, 2 => ""]); // AKdarray \/ AKdarray
  hh_show($x);
  hh_show($x[0]);

  $x = ($b ? darray[1 => true, 2 => false] : $arrayIntBool); // AKdarray \/ AKdarray
  hh_show($x);
  hh_show($x[0]);

  $x = ($b ? darray[1 => true, 2 => false] : darray[0 => true]); // AKdarray \/ AKdarray
  hh_show($x);
  hh_show($x[0]);

  $x = ($b ? $arrayIntBool : darray[0 => true]); // AKdarray \/ AKdarray
  hh_show($x);
  hh_show($x[0]);

  $x = ($b ? $arrayIntBool : $arrayIntBool2); // AKdarray \/ AKdarray
  hh_show($x);
  hh_show($x[0]);
}

function expect_array<T>(varray<T> $x): void {}
function expect_int_array(varray<int> $x): void {}
function expect_string_array(varray<string> $x): void {}
function expect_arraykey_array(varray<arraykey> $x): void {}
function expect_arraykey_varray(varray<arraykey> $x): void {}
function expect_arraykey_arraykey_array(darray<arraykey, arraykey> $x): void {}
function expect_arraykey_arraykey_darray(darray<arraykey, arraykey> $x): void {}
function expect_varray_array<T>(varray_or_darray<T> $x): void {}
