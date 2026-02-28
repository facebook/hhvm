<?hh

function x() :mixed{ return 2; }
function foo() :mixed{ return vec[x()]; }
function bar() :mixed{
  $z = foo();
  $z[] = 12;
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
function main_array_036() :mixed{
bar();
}
