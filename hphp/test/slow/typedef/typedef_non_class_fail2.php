<?hh

type Point = shape(
  'x' => float,
  'y' => float,
);

function woot(Point $x) : Point {
  return $x;
}

function test(): void {
  woot(null);  // failure, expected Point
}


<<__EntryPoint>>
function main_typedef_non_class_fail2() :mixed{
test();
}
