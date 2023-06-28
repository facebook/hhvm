<?hh

function f() :AsyncGenerator<mixed,mixed,void>{
  $a = (yield 1);
  $b = yield 2;
  list($c, $d) = (yield 3);
  list($e, $f) = yield 4;
}


<<__EntryPoint>>
function main_yield_parens() :mixed{
echo "OK\n";
}
