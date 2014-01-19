<?php

class a {
  function r(&$x) {
    $x = 20;
  }
}
function id($x) {
 return $x;
 }
$a = new a();
id($a)->r($x);
var_dump($x);
