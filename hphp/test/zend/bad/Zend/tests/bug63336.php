<?php
error_reporting(E_ALL | E_NOTICE );
define("TEST", "123");
class Base {
    const DUMMY = "XXX";
    public function foo($var=TEST, $more=null) { return true; }
    public function bar($more=self::DUMMY) { return true; }
}

class Child extends Base {
    const DUMMY = "DDD";
    public function foo($var=TEST, array $more = array()) { return true; }
    public function bar($var, $more=self::DUMMY) { return true; }
}
?>