<?php
error_reporting(E_ALL);

session_id("abtest");
session_start();

class a {
    public $test = "hallo";
}
 
class b {
    public $a;
    function b(&$a) {
        $this->a = &$a;
    }
}
 
$a = new a();
$b = new b($a);

echo "original values:\n";
var_dump($a,$b);

$_SESSION["a"] = $a;
$_SESSION["b"] = $b;
session_write_close();

unset($_SESSION["a"], $_SESSION["b"]);

session_start();

echo "values after session:\n";
var_dump($a,$b);
?>