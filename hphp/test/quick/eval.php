<?php

function main() {
  print "Test begin\n";

  $x = "Hello";
  $y = "world";
  $s = '$x $y!';
  for ($i = 0; $i < 2; ++$i) {
    eval("\$r = \"$s\";
         \$x = \"Goodbye\";");
    print "$i: $r\n";
  }

  function testEval(&$i) {
    eval('$i *= 33;');
  }
  $i = 1;
  while ($i < 100000) {
    testEval($i);
    var_dump($i);
  }

  for ($i = 0; $i < 5; ++$i) {
    eval('print "Hello!\n";');
  }

  print "Test end\n";
}
main();
