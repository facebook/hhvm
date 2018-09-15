<?hh

function test(): void {
  $y = 42;
  $z = &$y; // no error yet in partial mode
}
