--TEST--
Blank attribute
--FILE--
<?php
class xhp_a {}
$foo = <a b="" />;
echo "pass";
--EXPECT--
pass
