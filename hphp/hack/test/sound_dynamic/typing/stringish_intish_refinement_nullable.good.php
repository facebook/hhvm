<?hh

enum G: arraykey {
  MEANING = "42";
}

function takes_g(~?G $_): void {}

function test_opaque_is_arraykeyish(bool $b): void {
  $e = $b ? 42 : null;
  if ($e is ?G) {
    takes_g($e);
  }

  $e = $b ? "42" : null;
  if ($e is ?G) {
    takes_g($e);
  }
}

function test_opaque_as_arraykeyish(bool $b): void {
  $e = ($b ? 42 : null) as ?G;
  takes_g($e);

  $e = ($b ? "42" : null) as ?G;
  takes_g($e);
}

enum H: arraykey as arraykey {
  MEANING = "42";
}

function takes_h(~?H $_): void {}

function test_transparent_is_arraykeyish(bool $b): void {
  $e = $b ? 42 : null;
  if ($e is ?H) {
    takes_h($e);
  }

  $e = $b ? "42" : null;
  if ($e is ?H) {
    takes_h($e);
  }
}

function test_transparent_as_arraykeyish(bool $b): void {
  $e = ($b ? 42 : null) as ?H;
  takes_h($e);

  $e = ($b ? "42" : null) as ?H;
  takes_h($e);
}
