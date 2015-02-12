<?hh
type Coord = shape('x' => int);   // ok
type Coord = shape('x' => int,
                   'y' => int);   // ok: typechecker catches it
type Coord = string;              // bad
