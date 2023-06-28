<?hh

<<__EntryPoint>>
function main() :mixed{
  $x = new ArrayIterator(darray['a' => 'b']);
  array_key_exists('wat', $x);
}
