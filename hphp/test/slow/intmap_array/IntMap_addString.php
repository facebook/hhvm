<?php

function main() {
  $a = hphp_miarray();
  $a["string"] = 10;
  $a["anotherString"] = 20;
  var_dump($a);

  $a = hphp_miarray();
  $foo = "string";
  $a[$foo] = 10;
  $a[$foo] = "something";
  var_dump($a);
}

main();
