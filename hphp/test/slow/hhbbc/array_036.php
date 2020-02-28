<?hh

function x() { return 2; }
function foo() { return varray[x()]; }
function bar() {
  $z = foo();
  $z[1] = 12;
  for ($i = 0; $i < $z[1]; $z[1] = $z[1] - 1) {
    var_dump(is_int($z[0]));
    var_dump(is_int($z[1]));
    $z[0] = 'a';
    var_dump(is_int($z[0]));
    var_dump(is_int($z[1]));
  }
  var_dump(is_int($z[0]));
  var_dump(is_int($z[1]));
  var_dump($z);
}


<<__EntryPoint>>
function main_array_036() {
bar();
}
