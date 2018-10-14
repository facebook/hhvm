<?hh // strict

function test(): void {
  expect_lambda_returning_int(() ==> {
    // UNSAFE
    return "hey!";
  });

  $res1 = expect_lambda_generic("ho!", $x ==> {
    // UNSAFE
    return $x;
  });
  expect_int($res1); // error

  $res2 = expect_lambda_generic2(() ==> {
    // UNSAFE
    return "ho!";
  });
  expect_int($res2);

  $res3 = expect_lambda_generic2(() ==> {
    // UNSAFE
    return ident("ho!");
  });
  expect_int($res3);

  $res4 = expect_lambda_generic2(() ==> {
    // UNSAFE
    return make_string();
  });
  expect_int($res4);
}

function expect_lambda_returning_int((function (): int) $f): void {}

function expect_lambda_generic<T>(T $x, (function (T): T) $f): T {
  return $f($x);
}

function expect_lambda_generic2<T>((function (): T) $f): T {
  return $f();
}

function ident<T>(T $x): T {
  return $x;
}

function expect_int(int $x): void {}

function make_string(): string {
  return "hi!";
}
