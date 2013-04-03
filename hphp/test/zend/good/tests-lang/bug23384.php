<?php
define('TEN', 10);
class Foo {
    const HUN = 100;
    function test($x = Foo::HUN) {
        static $arr2 = array(TEN => 'ten');
        static $arr = array(Foo::HUN => 'ten');

        print_r($arr);
        print_r($arr2);
        print_r($x);
    }
}

Foo::test();   
echo Foo::HUN."\n";
?>