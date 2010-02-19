--TEST--
Reflection API 08
--FILE--
<?php
class foo {}
/** a */ class /** b */ bar /** c */ extends /** d */ foo /** e */ {}
$foo = new ReflectionClass('bar');
echo $foo->getDocComment();
exit;
<a />;
--EXPECT--
/** d */
