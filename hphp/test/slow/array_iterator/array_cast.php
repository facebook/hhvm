<?hh

<<__EntryPoint>>
function main() {
  $x = new ArrayIterator(darray['a' => 'b']);
  $x->p = 'q';
  var_dump(darray($x));
}
