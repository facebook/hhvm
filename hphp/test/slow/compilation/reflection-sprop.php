<?php

class asd {
  static $PROP = 2;
}

$y = (new ReflectionClass('asd'))->getProperty('PROP');
$y->setValue('asd');
var_dump(asd::$PROP);
