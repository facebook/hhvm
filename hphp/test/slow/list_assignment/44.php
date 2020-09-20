<?hh

function test($a) {
  list($a[0], $a[1], $a) = $a;
  var_dump($a);
}

<<__EntryPoint>>
function main_44() {
test(varray['abc', 'cde', 'fgh']);
}
