<?hh

function foo(): void {
  $a = 10;
  $b = $a ** 10; // num
  $c = 3 ** $b; // num
  expect_num($b);
  expect_num($c);

  $d = ($c === $a ** $b ** $c);
  expect_bool($d);

  $e = ((int)$d) ** 3.5;
  expect_float($e);
  $e = $e ** 100000;
  expect_float($e);

  $i = 1;
  $f = 3.5;

  $a = 1 ** 1.0; // float
  expect_float($a);
  $b = 1.0 ** 1; // float
  expect_float($b);
  $c = 1.0 ** 1.0; // float
  expect_float($c);

  expect_num(100000000 ** 10000000); // num
  expect_num(2 ** 2); // num
}

function expect_float(float $x): void {}
function expect_num(num $x): void {}
function expect_bool(bool $x): void {}
