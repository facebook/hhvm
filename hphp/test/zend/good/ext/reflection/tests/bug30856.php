<?php
class bogus {
        const C = 'test';
        static $a = bogus::C;
}

$class = new ReflectionClass('bogus');

var_dump($class->getStaticProperties());
?>
===DONE===
