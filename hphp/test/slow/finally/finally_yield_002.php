<?hh

function bleh() :AsyncGenerator<mixed,mixed,void>{
  echo "begin\n";
  try {
    echo "try\n";
    yield 1;
    echo "try\n";
    yield 2;
  } finally {
    echo "finally\n";
  }
  echo "end\n";
  yield 3;
}

function main() :mixed{
  $xs = bleh();

  foreach ($xs as $x) {
    echo "received: $x\n";
  }
}



<<__EntryPoint>>
function main_finally_yield_002() :mixed{
main();
}
