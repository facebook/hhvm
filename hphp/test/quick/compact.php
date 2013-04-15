<?php

function main() {
  $a = array(1, 2, "foo");
  $b = array("bar", $a);
  $c = &$b;
  $d = array(&$c);
  $e = 1;
  print_r(compact("b", "a", array("c", "d"), "e", "f"));
}
main();

