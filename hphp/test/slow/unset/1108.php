<?php

Unset1108::$a = array(10);
 function test() {

 unset(Unset1108::$a[0]);
 var_dump(Unset1108::$a);
}
var_dump(Unset1108::$a);
 test();
 var_dump(Unset1108::$a);

abstract final class Unset1108 {
  public static $a;
}
