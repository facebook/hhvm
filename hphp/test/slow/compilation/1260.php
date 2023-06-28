<?hh

function test() :mixed{
  $a = 0;
  try {
    $a += $b;
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1260() :mixed{
  test();
  echo "done\n";
}
