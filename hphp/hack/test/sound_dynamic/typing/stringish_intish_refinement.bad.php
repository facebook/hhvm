<?hh

enum E: string {
  MEANING = "42";
}

function takes_e(E $_): void {}

function test_opaque_is_stringish(): void {
  $e = 42;
  if ($e is E) {
    takes_e($e);
  }

  $e = "42";
  if ($e is E) {
    takes_e($e);
  }
}

function test_opaque_as_stringish(): void {
  $e = 42 as E;
  takes_e($e);

  $e = "42" as E;
  takes_e($e);
}

enum F: string as string {
  MEANING = "42";
}

function takes_f(F $_): void {}

function test_transparent_is_stringish(): void {
  $e = 42;
  if ($e is F) {
    takes_f($e);
  }

  $e = "42";
  if ($e is F) {
    takes_f($e);
  }
}

function test_transparent_as_stringish(): void {
  $e = 42 as F;
  takes_f($e);

  $e = "42" as F;
  takes_f($e);
}
