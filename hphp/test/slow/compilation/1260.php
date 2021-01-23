<?hh

function test() {
  $a = 0;
  try {
    $a += $b;
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1260() {
  test();
  echo "done\n";
}
