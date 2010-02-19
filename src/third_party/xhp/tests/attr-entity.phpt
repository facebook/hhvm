--TEST--
Entities in attributes
--FILE--
<?php
class xhp_a {}
$foo = <a b="&snowman;">c</a>;
echo "pass";
--EXPECT--
pass
