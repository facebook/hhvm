<?hh

function gen(int $mode) :AsyncGenerator<mixed,mixed,void>{
  yield $mode;
  switch ($mode) {
    case 0: break;
    case 1: yield break;
    case 2: throw new Exception();
  }
  yield 47;
}


<<__EntryPoint>>
function main_2185() :mixed{
for ($mode = 0;
 $mode < 3;
 ++$mode) {
  echo "Testing mode $mode:\n";
  $gen = gen($mode);
  try {
    $gen->rewind();
    while ($gen->valid()) {
      var_dump($gen->current());
      $gen->next();
    }
  }
 catch (Exception $ex) {
    echo "EXCEPTION\n";
  }
  var_dump($gen->valid());
  var_dump($gen->current());
}
}
