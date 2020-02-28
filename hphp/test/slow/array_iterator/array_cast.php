<?hh

<<__EntryPoint>>
function main() {
  $x = new ArrayIterator(darray['a' => 'b']);
  $x->p = 'q';
  var_dump((array)$x);
  $x = new ArrayIterator(darray['a' => 'b'],  ArrayIterator::STD_PROP_LIST);
  $x->p = 'q';
  var_dump((array)$x);
}
