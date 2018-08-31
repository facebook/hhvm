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


<<__EntryPoint>>
function main_jmp_type_002() {
main(12);
main(array(1,2,3));
}
