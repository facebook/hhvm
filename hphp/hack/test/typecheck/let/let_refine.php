<?hh // experimental

function expect_non_null(int $i): void {}

function foo(?int $maybe_int): void {
  let bar : ?int = $maybe_int;
  if (bar !== null) {
    expect_non_null(bar);
  }
}
