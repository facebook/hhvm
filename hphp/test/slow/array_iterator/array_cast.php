<?hh

<<__EntryPoint>>
function main() {
  $x = new ArrayIterator(array('a' => 'b'));
  $x->p = 'q';
  var_dump((array)$x);
  $x = new ArrayIterator(array('a' => 'b'),  ArrayIterator::STD_PROP_LIST);
  $x->p = 'q';
  var_dump((array)$x);
}
