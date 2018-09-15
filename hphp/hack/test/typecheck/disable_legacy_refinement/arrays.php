<?hh // strict

function f(mixed $x): void {
  if (is_dict($x)) {
    expect_dict($x);
  }
  if (is_keyset($x)) {
    expect_keyset($x);
  }
  if (is_vec($x)) {
    expect_vec($x);
  }
}

function expect_dict(dict<arraykey, mixed> $x): void {}
function expect_keyset(keyset<arraykey> $x): void {}
function expect_vec(vec<mixed> $x): void {}
