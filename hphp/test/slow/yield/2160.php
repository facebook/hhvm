<?hh

function f($x) :mixed{
 return $x;
 }
function foo($a) :AsyncGenerator<mixed,mixed,void>{
  yield 1;
  foreach (darray(f($a) ) as $x) {
    var_dump('i:'.$x);
  }
}

<<__EntryPoint>>
function main_2160() :mixed{
foreach (foo(vec[1]) as $x) {
  var_dump('o:'.$x);
}
}
