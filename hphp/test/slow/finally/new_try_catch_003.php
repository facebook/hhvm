<?hh

function blah() :mixed{
  $xs = vec[1, 2, 3];
  $ys = vec[1, 2, 3];
  $zs = vec[1, 2, 3];
  try {
    foreach ($xs as $x) {
      foreach ($ys as $y) {
        try {
          foreach ($zs as $z) {
            echo "$x\n";
            echo "$y\n";
            echo "$z\n";
            if ($x == 2 && $y == 2 && $z == 2) {
              echo "4\n";
              throw new Exception("7");
              echo "aaa\n";
            }
          }
        } finally {
          echo "$x\n";
          echo "$y\n";
          echo "$z\n";
          echo "5\n";
        }
      }
    }
  } finally {
    echo "$x\n";
    echo "$y\n";
    echo "$z\n";
    echo "6\n";
  }
}
<<__EntryPoint>> function main(): void {
var_dump(blah());
}
