<?hh

function blah() :mixed{
  $xs = vec[11, 22, 33, 44, 55];
  $ys = vec['a', 'b', 'c', 'd', 'e', 'f'];

  foreach ($ys as $y) {
    echo "begin outer loop $y\n";
    try {
      try {
        foreach ($xs as $x) {
          echo "begin inner loop $x\n";
          if ($x == 22) {
            echo "continue\n";
            continue;
          }
          echo "end inner loop $x\n";
        }
      } finally {
        echo "inner finally\n";
      }
    } finally {
      echo "outer finally\n";
    }
    echo "middle outer loop\n";
    try {
      try {
        $continue_after_loop = false;
        foreach ($xs as $x) {
          echo "begin inner loop $x\n";
          if ($x == 22 && $y == 'b') {
            echo "continue 2\n";
            $continue_after_loop = true;
            break;
          }
          if ($x == 33) {
            echo "break\n";
            break;
          }
          echo "end inner loop $x\n";
        }
        if ($continue_after_loop) continue;
      } finally {
        echo "inner finally 2\n";
      }
    } finally {
      echo "outer finally 2\n";
    }
    echo "end outer loop\n";
  }
}



<<__EntryPoint>>
function main_finally_break_005() :mixed{
blah();
}
