<?hh

class Foo {}
function foo($x) {
  if ($x) {
    $a = varray[];
    $s = 'hello';
    $o = new Foo();
  }
  var_dump((array)$a, (array)$s, (array)$o);
  var_dump((string)$a, (string)$s, (string)$o);
}

<<__EntryPoint>>
function main_1438() {
foo(false);
}
