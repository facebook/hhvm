<?php
class Crasher extends DateInterval {
    public $foo;
    public function __construct($time_spec) {
        var_dump($this->foo);
        $this->foo = 3;
        var_dump($this->foo);
        var_dump($this->{2});
        parent::__construct($time_spec);
    }
}
try {
    $c = new Crasher('blah');
} catch (Exception $e) {
    var_dump($e->getMessage());
}