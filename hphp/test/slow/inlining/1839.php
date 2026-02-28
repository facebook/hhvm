<?hh

function inline_me($x, $y, inout $z) :mixed{
 $z = ($x + $y);
 return ($z );
 }
function gen($x, $y) :AsyncGenerator<mixed,mixed,void>{
  $arg = null;
  yield inline_me($x, $y, inout $arg);
  yield $arg;
}

<<__EntryPoint>>
function main_1839() :mixed{
foreach (gen(10, 20) as $x) {
 var_dump($x);
 }
}
