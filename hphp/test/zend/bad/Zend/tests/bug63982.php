<?php
class Test {
        protected $protectedProperty;
}

$test = new Test();

var_dump(isset($test->protectedProperty));
var_dump(isset($test->protectedProperty->foo));