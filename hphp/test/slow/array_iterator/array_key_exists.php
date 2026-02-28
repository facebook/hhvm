<?hh

<<__EntryPoint>>
function main() :mixed{
  $x = new ArrayIterator(dict['a' => 'b']);
  array_key_exists('wat', $x);
}
