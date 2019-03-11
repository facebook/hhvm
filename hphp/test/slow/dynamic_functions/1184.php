<?php

DynamicFunctions1184::$a = 'test';
 function test() {

 return DynamicFunctions1184::$a;
}
 $b = (DynamicFunctions1184::$a)();
 $b = 'ok';
 var_dump(DynamicFunctions1184::$a);

abstract final class DynamicFunctions1184 {
  public static $a;
}
