<?hh

function test() :mixed{
  try {
    if ($y is ?int) echo "Yes\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_uninit_vs_nullable() :mixed{
  test();
}
