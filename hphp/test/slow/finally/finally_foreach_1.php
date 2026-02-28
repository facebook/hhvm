<?hh
function foo() :mixed{
  foreach (vec[1,2,3] as $x) {
    try {
      echo "A\n";
      throw new Exception("c");
    } finally {
      echo "B\n";
      foreach (vec[1,2,3] as $y) { var_dump($y); }
    }
  }
}
function main() :mixed{
  try {
    foo();
  } catch (Exception $e) {
    echo "Caught exception\n";
  }
}

<<__EntryPoint>>
function main_finally_foreach_1() :mixed{
main();
}
