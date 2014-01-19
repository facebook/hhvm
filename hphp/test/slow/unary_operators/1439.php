<?php

function test($x) {
  switch ($x) {
    case 1:      $y = true;
    case 2:      var_dump(isset($y), $y);
  }
}
test(2);
