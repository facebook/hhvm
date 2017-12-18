<?hh

function f(inout string $s): void {}

function test(): void {
  $foo = 'bar';
  $baz = 'foo';
  /* HH_FIXME[1002] ignore the parse error */
  f(inout $$baz);
}
