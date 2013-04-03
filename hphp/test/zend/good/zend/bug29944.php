<?php
$a = 1;
switch ($a) {
  case 1:
    function foo($a) {
      return "ok\n";
    }
    echo foo($a);
}
?>