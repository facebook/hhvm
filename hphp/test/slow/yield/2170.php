<?hh

function bar($x) :mixed{
 return $x ? $x + 1 : false;
 }
function foo($a) :AsyncGenerator<mixed,mixed,void>{
  $x = bar($a);
  switch ($x) {
    case 'hello': echo 1;
 break;
    case bar(3): echo 2;
 break;
  }
  yield $x;
}

<<__EntryPoint>>
function main_2170() :mixed{
foreach(foo(3) as $x) {
 var_dump($x);
 }
}
