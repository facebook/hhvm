<?hh

function gen() {
  yield "foo" => 0;
  yield 1     => 1;
  yield 2.5   => 2;
  yield null  => 3;
  yield varray[] => 4;
  yield new stdClass => 5;
}
<<__EntryPoint>>
function main(): void {
  var_dump(iterator_to_array(gen()));
}
