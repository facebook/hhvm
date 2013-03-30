<?php 

interface iTest { }
 
class baz implements iTest {}
 
class bar { }
 
class foo extends bar {
    public function testFoo(self $obj) {
        var_dump($obj);
    }
    public function testBar(parent $obj) {
        var_dump($obj);
    }
    public function testBaz(iTest $obj) {
        var_dump($obj);
    }
}
 
$foo = new foo;
$foo->testFoo(new foo);
$foo->testBar(new bar);
$foo->testBaz(new baz);
$foo->testFoo(new stdClass); // Catchable fatal error

?>