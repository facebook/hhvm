<?hh

function asd() :mixed{ return vec[]; }
function foo() :mixed{
  $x = asd();
  foreach ($x as $v) {
    echo "unreachable code: $v\n";
  }
  return 1;
}

<<__EntryPoint>>
function main_iter_005() :mixed{
foo();
}
