<?hh

function asd() :mixed{ return varray[]; }
function foo() :mixed{
  $x = asd();
  foreach ($x as $k => $v) {
    echo "unreachable code: $k $v\n";
  }
  return 1;
}

<<__EntryPoint>>
function main_iter_004() :mixed{
foo();
}
