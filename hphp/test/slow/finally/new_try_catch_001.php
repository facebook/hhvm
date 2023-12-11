<?hh

function blah() :mixed{
  $xs = vec[1, 2, 3];
  $ys = vec[1, 2, 3];
  $zs = vec[1, 2, 3];
  try {
    foreach ($xs as $x) {
      foreach ($ys as $y) {
        foreach ($zs as $z) {
          echo "$x\n";
          echo "$y\n";
          echo "$z\n";
          if ($x == 2 && $y == 2 && $z == 2) {
            echo "4\n";
            throw new Exception("blah!");
            echo "aaa\n";
          }
        }
      }
    }
  } catch (Exception $e) {
    echo (!($x ?? false)) ? "bbb\n" : "5\n";
    echo (!($y ?? false)) ? "ccc\n" : "6\n";
    echo (!($z ?? false)) ? "ddd\n" : "7\n";
    echo "8\n";
  }
  return 9;
}



<<__EntryPoint>>
function main_new_try_catch_001() :mixed{
var_dump(blah());
}
