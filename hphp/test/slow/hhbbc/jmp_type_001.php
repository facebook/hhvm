<?php

class Foo {}
class Bar {}

function main($x) {
  if (is_int($x)) {
    echo is_int($x);
    echo "\n";
  } else {
    if (is_array($x)) {
      echo is_array($x);
      echo (bool)$x;
      echo "\n";
    }
  }
}


<<__EntryPoint>>
function main_jmp_type_001() {
main(12);
main(array(1,2,3));
}
