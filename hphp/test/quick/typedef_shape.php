<?hh

type Point = shape(
  'x' => int,
  'y' => int,
);

function foo(Point $pt): void {
  var_dump($pt);
}

function main(): void {
  foo(shape(
    'x' => 12,
    'y' => 12
  ));
}

main();
