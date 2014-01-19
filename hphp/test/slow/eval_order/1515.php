<?php

function f($a) {
 echo "test$a\n";
 return 1;
 }
function bug2($a, $b) {
  return isset($b[f($a++)], $b[f($a++)], $b[f($a++)]);
}
bug2(0, array());
