<?hh

<<__EntryPoint>>
function main() {
  $x = new ArrayIterator(darray['a' => 'b']);
  array_key_exists('wat', $x);
}
