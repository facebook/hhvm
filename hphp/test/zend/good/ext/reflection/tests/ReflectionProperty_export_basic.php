<?php

class TestClass {
    public $proper = 5;
}

var_dump(ReflectionProperty::export('TestClass', 'proper'));

?>
