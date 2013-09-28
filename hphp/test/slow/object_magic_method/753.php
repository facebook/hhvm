<?php

class foo{
  public $public = 'public';
  public function __sleep()  {
 return array('public');
 }
}
$foo = new foo();
$data = serialize($foo);
var_dump($data);
