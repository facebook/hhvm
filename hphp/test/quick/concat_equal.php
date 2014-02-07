<?php

function main($b) {
  $a .= $b;
  var_dump($a);
}
class b { function __toString() { return 'b'; }}

main(new b);
