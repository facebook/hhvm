<?hh

function main(mixed $x): void {
  if ($x is arraykey) {
    expect_arraykey($x);
  } else if ($x is num) {
    expect_num($x);
  }
  expect_arraykey($x);
  expect_num($x);
}

function expect_arraykey(arraykey $x): void {}
function expect_num(num $x): void {}
