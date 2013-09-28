<?php

class Base{
  public    $aaa = 1;
  protected $bbb = 2;
  private   $ccc = 3;
  function foo($obj) {
    var_dump(get_class($obj));
    var_dump(get_object_vars($obj));
  }
}
class Child extends Base{
  public    $ddd = 5;
  protected $eee = 6;
  private   $fff = 4;
}
$base_obj = new Base();
$base_obj->foo($base_obj);
