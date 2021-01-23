<?hh

function test() {
  try {
    if (@$y is ?int) echo "Yes\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_uninit_vs_nullable() {
  test();
}
