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


<<__EntryPoint>>
function main_typedef_non_class_fail2() {
test();
}
