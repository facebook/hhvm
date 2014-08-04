<?php
function main() {
  $x = hphp_varray();
  $y = $x['foo'];
  $x['foo'] = 1;
  var_dump($x);
}
main();
