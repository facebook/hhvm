<?hh // partial

function f(inout string $s): void {}

function test(): void {
  $foo = 'bar';
  $baz = 'foo';
  /* HH_FIXME[2081] ignore the variable-variable error and pretend it's legal */
  f(inout $$baz);
}
