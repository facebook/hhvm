<?hh

function test() {
  $a = varray['abc', 'cde', 'fgh'];
  list($a[0], $a[1], $a) = $a;
  var_dump($a);
}

<<__EntryPoint>>
function main_45() {
test();
}
