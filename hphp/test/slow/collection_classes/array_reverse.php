<?hh

function reverse($input) :mixed{
  echo "--------------------\n";
  echo "input: ";
  var_dump($input);
  echo "reverse discard keys: ";
  $reverse_discard = array_reverse($input);
  var_dump($reverse_discard);
  var_dump(array_reverse(darray($input)) === $reverse_discard);
  echo "reverse preserve keys: ";
  $reverse_preserve = array_reverse($input, true);
  var_dump($reverse_preserve);
  var_dump(array_reverse(darray($input), true) === $reverse_preserve);
}

function main() :mixed{
  reverse(vec['foo', 'bar', 'baz']);
  reverse(dict['a' => 'foo', 'b' => 'bar', 'c' => 'baz']);
  reverse(dict[10 => 'foo', 20 => 'bar', 30 => 'baz']);
  reverse(Vector {'foo', 'bar', 'baz'});
  reverse(Set {'foo', 'bar', 'baz'});
  reverse(Map { 'a' => 'foo', 'b' => 'bar', 'c' => 'baz'});
  reverse(Map { 10 => 'foo', 20 => 'bar', 30 => 'baz'});
  reverse(ImmVector {'foo', 'bar', 'baz'});
  reverse(ImmSet {'foo', 'bar', 'baz'});
  reverse(ImmMap { 'a' => 'foo', 'b' => 'bar', 'c' => 'baz'});
  reverse(ImmMap { 10 => 'foo', 20 => 'bar', 30 => 'baz'});
}


<<__EntryPoint>>
function main_array_reverse() :mixed{
main();
}
