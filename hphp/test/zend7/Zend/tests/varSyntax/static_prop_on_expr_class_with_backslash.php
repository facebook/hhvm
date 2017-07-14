<?php

class A {
    public static $b = 42;
}
var_dump(('\A' . (string) '')::$b);

?>
