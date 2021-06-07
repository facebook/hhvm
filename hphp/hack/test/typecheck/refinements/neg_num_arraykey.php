<?hh

function test(bool $b, num $v): void {
  if ($b) {
    $v = 'a';
  }
  if ($v is arraykey) {
    expect_string($v); // error
    expect_float($v); // error
    expect_int($v); // error
    expect_num($v); // error
    expect_arraykey($v);
  }
  else {
    expect_string($v); // error
    expect_float($v);
    expect_int($v); // error
    expect_num($v);
    expect_arraykey($v); // error
  }
}

function test2(bool $b, arraykey $v): void {
  if ($b) {
    $v = 1.0;
  }
  // $v : float | arraykey

  if ($v is num) {
    expect_string($v); // error
    expect_float($v); // error
    expect_int($v); // error
    expect_num($v);
    expect_arraykey($v); // error
  }
  else {
    expect_string($v);
    expect_float($v); // error
    expect_int($v); // error
    expect_num($v); // error
    expect_arraykey($v);
  }
}


function expect_string(string $in): void {}
function expect_int(int $in): void {}
function expect_float(float $in): void {}
function expect_arraykey(arraykey $in): void {}
function expect_num(num $in): void {}
