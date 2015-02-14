<?hh

type Something = int;
type Something = int;
type Something = int;

type Coord = (int, int);
type Coord = (int, int, int); // only caught by typechecker

type Foo = shape(
  'x' => int,
);
type Foo = shape(   // only caught by typechecker
  'x' => string,
);

class Something2 {}
type SomethingElse = Something2;
type SomethingElse = Something2;

echo "Yeah\n";
