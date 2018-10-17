<?hh // strict

function test1(): void {
  while (true) {
    // UNSAFE
    $x = 1;
    break;
  }
  expect_string($x);
}

function test2(bool $b): void {
  do {
    if ($b) {
      // UNSAFE
      $x = 1;
      break;
    }
    $x = "";
  } while ($b);
  expect_string($x);
}

function test3(bool $b): void {
  do {
    if ($b) {
      // UNSAFE
      break;
    }
    $x = "";
  } while ($b);
  expect_string($x);
}

function test4(vec<int> $v): void {
  foreach ($v as $x) {
    // UNSAFE
    $f = $y ==> f();
  }
}

function f(): void {}
function expect_string(string $x): void {}
