<?hh

<<__EntryPoint>>
function main() {
  $x = new ArrayObject();
  $x->storage = $x;
  var_dump((array)$x);
}
