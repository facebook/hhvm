<?hh //strict

function with_and(?int $x, ?int $s): void {
  if (($y = $x) && ($z = $y) && ($u = $s)) {
    expect_int($y);
    expect_int($z); // $z is defined, otherwise we wouldn't have reached here
    expect_int($u); // same for $u
  } else if ($t = $s) {
    expect_nullint($y); // ok, $y is still defined
    expect_int($y); // error, $y may be null
    expect_nullint($z); // error, $z might be undefined
    expect_nullint($u); // error, $u might be undefined
    expect_int($t); // ok, $t is defined
  }
  expect_nullint($y); // ok, $y is still defined
  expect_int($y); // error, $y may be null
  expect_nullint($z); // error, $z might be undefined
  expect_nullint($u); // error, $u might be undefined
  expect_nullint($t); // error, $t might be undefined
}

function with_or(?int $x, ?int $s): void {
  if (!(($y = $x) || ($z = $y) || ($u = $s))) {
    expect_nullint($y);
    expect_nullint($z); // $z is defined, otherwise we wouldn't have reached here
    expect_nullint($u); // same for $u
  } else if ($t = $s) {
    expect_nullint($y); // ok, $y is still defined
    expect_nullint($z); // error, $z might be undefined
    expect_nullint($u); // error, $u might be undefined
    expect_int($t); // ok, $t is defined
  }
  expect_nullint($y); // ok, $y is still defined
  expect_nullint($z); // error, $z might be undefined
  expect_nullint($u); // error, $u might be undefined
  expect_nullint($t); // error, $t might be undefined
}

function expect_int(int $x): void {}
function expect_nullint(?int $x): void {}

// Now happily play around with hhvm :)
//f(0, 0);
//f(0, 1);
//f(1, 0);
//f(1, 1);
