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
            echo "break\n";
            break;
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
        $break_outer_loop = false;
        foreach ($xs as $x) {
          echo "begin inner loop $x\n";
          if ($x == 33 && $y == 'e') {
            echo "break 2\n";
            $break_outer_loop = true;
            break;
          }
          if ($x == 33) {
            echo "break\n";
            break;
          }
          echo "end inner loop $x\n";
        }
        if ($break_outer_loop) break;
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
function main_finally_break_004() :mixed{
blah();
}
