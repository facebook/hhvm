<?hh // partial

/**
 * TODO(T29281383)
 */
function main($x): void {
  if ($x is int) {
    expect_int($x);
    expect_string($x);
    expect_resource($x);
  }
}

function expect_int(int $x): void {}
function expect_string(string $x): void {}
function expect_resource(resource $x): void {}
