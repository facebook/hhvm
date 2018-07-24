<?hh

type Foo = shape(
  'a_abc' => string,
  'a_def' => string,
  'b_abc' => int,
);

function foo(Foo $in): void {
  var_dump($in[AUTO332
