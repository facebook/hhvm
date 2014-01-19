<?php

trait MY_TRAIT {
 }
class MY_CLASS {
 use MY_TRAIT;
 }
$r = new ReflectionClass('MY_CLASS');
var_dump($r->getTraitNames());
?>
