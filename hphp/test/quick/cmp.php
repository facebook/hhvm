<?hh

<<__EntryPoint>> function main(): void {
  $arr = vec[null, false, true, 0, 1, 2, -1, "", "0", "0.0", "1", "1.0",
               "2", "2.0", "-1", "-1.0", "-0", "-0.0", "a", "\000", "a\000",
               "b\000", "\000a", "\000b", "0\000"];
  foreach ($arr as $left) {
    foreach ($arr as $right) {
      echo "left: " . (string)(var_dump($left));
      echo "right: " . (string)(var_dump($right));
      echo "==  "; echo HH\Lib\Legacy_FIXME\eq($left, $right) ? "true\n" : "false\n";
      echo "!=  "; echo HH\Lib\Legacy_FIXME\neq($left, $right) ? "true\n" : "false\n";
      echo "=== "; echo $left === $right ? "true\n" : "false\n";
      echo "!== "; echo $left !== $right ? "true\n" : "false\n";
      echo "\n";
    }
  }
}
