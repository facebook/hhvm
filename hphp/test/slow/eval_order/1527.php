<?php

function f() {

 return ++EvalOrder1527::$a;
 }
var_dump(array(EvalOrder1527::$a,f(),EvalOrder1527::$a));

abstract final class EvalOrder1527 {
  public static $a;
}
