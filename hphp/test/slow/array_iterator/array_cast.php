<?hh

<<__EntryPoint>>
function main() :mixed{
  $x = new ArrayIterator(dict['a' => 'b']);
  $x->p = 'q';
  var_dump(darray($x));
}
