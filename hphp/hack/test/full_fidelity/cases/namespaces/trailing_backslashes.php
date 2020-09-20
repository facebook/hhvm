<?hh

// Qualified names should not have trailing backslashes

function test(): void {
  $x = new Foospace\();

  $y = Foospace\bar\();

  $z = Foo\::class;
}
