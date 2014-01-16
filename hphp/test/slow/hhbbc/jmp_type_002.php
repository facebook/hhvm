<?php

class Foo {}
class Bar {}

function main($x) {
  if (!is_array($x)) $x = array($x);
  echo is_array($x);
  echo "\n";
  echo (bool)$x;
  echo "\n";
}

main(12);
main(array(1,2,3));
