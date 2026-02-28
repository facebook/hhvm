<?hh

function main(int $x): void {
  if ($x is dynamic) {
    expect_int($x);
    expect_dynamic($x);
  }
}

function expect_int(int $x): void {}
function expect_dynamic(dynamic $x): void {}
