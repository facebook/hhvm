--TEST--
Presumptous Closing Tags
--FILE--
<?php
class xhp_a {}
$foo = <a><a><a>hi</a></></a>;
echo "pass";
--EXPECT--
pass
