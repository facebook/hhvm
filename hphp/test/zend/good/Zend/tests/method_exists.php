<?php
class testclass { function testfunc() { } }
var_dump(method_exists('testclass','testfunc'));
var_dump(method_exists('testclass','nonfunc'));
?>