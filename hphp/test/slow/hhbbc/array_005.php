<?hh

type counter = shape(
  'name'  => string,
  'count' => int,
);

function make_counter(string $x, int $y): counter {
  return shape('name' => $x, 'count' => $y);
}

function test(bool $x): void {
  $foo = make_counter('foo', 12);
  $bar = make_counter('bar', 42);
  $z = $x ? $foo : $bar;
  return array($z['name'], $z['count']);
}

function main() {
  list($name, $cnt) = test(true);
  var_dump($name, $cnt);
  list($name, $cnt) = test(false);
  var_dump($name, $cnt);
}

main();
