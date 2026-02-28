<?hh

enum E: string {
  MEANING = "42";
}

function takes_e(?E $_): void {}

function test_opaque_is_stringish(bool $b): void {
  $e = $b ? 42 : null;
  if ($e is ?E) {
    takes_e($e);
  }

  $e = $b ? "42" : null;
  if ($e is ?E) {
    takes_e($e);
  }
}

function test_opaque_as_stringish(bool $b): void {
  $e = ($b ? 42 : null) as ?E;
  takes_e($e);

  $e = ($b ? "42" : null) as ?E;
  takes_e($e);
}

enum F: string as string {
  MEANING = "42";
}

function takes_f(?F $_): void {}

function test_transparent_is_stringish(bool $b): void {
  $e = $b ? 42 : null;
  if ($e is ?F) {
    takes_f($e);
  }

  $e = $b ? "42" : null;
  if ($e is ?F) {
    takes_f($e);
  }
}

function test_transparent_as_stringish(bool $b): void {
  $e = ($b ? 42 : null) as ?F;
  takes_f($e);

  $e = ($b ? "42" : null) as ?F;
  takes_f($e);
}
