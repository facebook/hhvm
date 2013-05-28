<?php

class Base{
  public    $aaa = 1;
  protected $bbb = 2;
  private   $ccc = 3;
}
class Child extends Base{
  private   $ddd = 4;
}
var_dump(get_object_vars(new Base()));
var_dump(get_object_vars(new Child()));
