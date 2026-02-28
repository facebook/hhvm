<?hh

type Supertype = shape(?'a' => int);
type Subtype = shape('a' => int);

function consumesSupertype(Supertype $argument): void {}

function providesSubtype(): Subtype {
  return shape('a' => 0);
}

function test(): void {
  consumesSupertype(providesSubtype());
}
