<?hh
function foo() :mixed{
  foreach (vec[1,2,3] as $v) {
    foreach (vec[1,2,3] as $w) {
      try {
        echo "A\n";
        throw new Exception("c");
      } finally {
        echo "B\n";
        foreach (vec[1,2,3] as $x) {
          foreach (vec[1,2,3] as $y) {
            try {
              echo "C\n";
              throw new Exception("d");
            } finally {
              echo "D\n";
              foreach (vec[1,2,3] as $z) { var_dump($z); }
            }
          }
        }
      }
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
function main_finally_foreach_2() :mixed{
main();
}
