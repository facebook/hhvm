<?hh

type Point = shape(
  'x' => real,
  'y' => real,
);

function woot(Point $x) : Point {
  return $x;
}

function test(): void {
  woot(null);  // failure, expected Point
}

test();
