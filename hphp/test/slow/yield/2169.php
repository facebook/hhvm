<?hh

function foo($x = null) :AsyncGenerator<mixed,mixed,void>{
  if ($x) $x = 'foo';
  var_dump($x);
  yield 1;
  }

<<__EntryPoint>>
function main_2169() :mixed{
foreach(foo() as $x) {
}
}
