<?php

class ExtendedArrayObject extends ArrayObject {
  public function offsetSet($key, $value) {
    $key = 'q1';
    parent::offsetSet($key, $value);
  }
}

$arrayobj = new ArrayObject(array('first','second','third'));
$arrayobj->append('fourth');
$arrayobj->append(array('five', 'six'));
var_dump($arrayobj);

$arrayobj = new ExtendedArrayObject(array('y'));
$arrayobj->append('x');
var_dump($arrayobj);

$arrayobj = new ExtendedArrayObject(array('q2' => 'y'));
$arrayobj->append('z');
var_dump($arrayobj);
