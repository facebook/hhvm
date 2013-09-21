<?php
class Stat {
    private static $requests;
    public static function getInstance() {
        if (!isset(self::$requests[1])) {
            self::$requests[1] = new self();
        }
        return self::$requests[1];
    }
    
    public function __destruct() {
        unset(self::$requests[1]);
    }
}

class Foo {
    public function __construct() {
        Stat::getInstance();
    }
}

class Error {
    private $trace;
    public function __construct() {
        $this->trace = debug_backtrace(1);
    }
}

class Bar {
    public function __destruct() {
        Stat::getInstance();
        new Error();
    }

    public function test() {
        new Error();
    }
}

$foo = new Foo();
$bar = new Bar();
$bar->test();
?>