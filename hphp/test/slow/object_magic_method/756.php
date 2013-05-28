<?php

class foo{
  public $public = 'public';
  public function __wakeup()  {
 echo "__wakeup called\n";
 return 1;
 }
}
$foo = unserialize("O:3:\"foo\":1:{s:6:\"public\";s:6:\"public\";}");
var_dump($foo);
