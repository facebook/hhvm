<?hh

<<__EntryPoint>>
function main() {
  $x = new ArrayObject(array('a' => 'b'));
  $x->p = 'q';
  var_dump((array)$x);
  $x = new ArrayObject(array('a' => 'b'),  ArrayObject::STD_PROP_LIST);
  $x->p = 'q';
  var_dump((array)$x);
}
