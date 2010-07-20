--TEST--
Syntax Error Line Number
--FILE--
<?php
$foo = xhp_preprocess_code('<?php
$foo = <x><y></x>;









function foo() {}');
echo $foo['error_line'];
--EXPECT--
2
