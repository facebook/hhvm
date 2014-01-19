<?php ;

error_reporting(0);

function test() {
  for ($i = 0; $i < 10000; $i++) {
    strtotime("10 September 2000");
    strtotime("10 September 2000 UTC");
    strtotime("null");
  }
}

function main() {
  $a = memory_get_usage(true);
  test();
  $b = memory_get_usage(true);
  test();
  $c = memory_get_usage(true);
  $v1 = $b - $a;
  $v2 = ($c - $b) * 10;
  if ($v2 <= $v1) {
    echo "Ok\n";
  } else {
    echo "strtotime is leaking: $a, $b, $c\n";
  }
}

var_dump(strtotime("10 September 2000 UTC"));
var_dump(strtotime("null"));

main();
