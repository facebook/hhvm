<?hh

function takes_arraykey(arraykey $_): void {}

enum E: string {}

function main(): void {
  $e = 42 as E; // : ~E & arraykey

  // The following two typecheck due to the & arraykey part of $e's type.
  takes_arraykey($e);
  keyset(vec[$e]);
}
