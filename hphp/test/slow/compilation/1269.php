<?php

class A {
 static $a = 1;
}
 class B extends A {
 static $a = 2;
}
 var_dump(B::$a);
