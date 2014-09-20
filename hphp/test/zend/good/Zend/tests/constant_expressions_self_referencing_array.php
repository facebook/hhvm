<?php
class A {
   const FOO = [self::BAR];
   const BAR = [self::FOO];
}
var_dump(A::FOO);
?>
