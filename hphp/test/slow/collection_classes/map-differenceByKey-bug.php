<?php
function main() {
  $x = Map {};
  $y = Map {'a' => 1, 'b' => 2};
  $x->differenceByKey($y);
  var_dump($x);
}
main();
