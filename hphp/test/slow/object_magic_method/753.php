<?php

class foo{
  public $public = 'public';
  public function __sleep()  {
 return array('public');
 }
}

<<__EntryPoint>>
function main_753() {
$foo = new foo();
$data = serialize($foo);
var_dump($data);
}
