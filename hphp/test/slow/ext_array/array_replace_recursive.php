<?php

$ar1 = array(
  "color" => array("favoritte" => "red"),
  5
);
$ar2 = array(
  10,
  array("color" => array("favorite" => "green", "blue"))
);
$r = array_replace_recursive($ar1, array($ar2));
var_dump($r);
