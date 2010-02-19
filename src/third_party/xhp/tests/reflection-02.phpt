--TEST--
Reflection API 02
--FILE--
<?php
/** a */ abstract /** b */ class /** c */ foo /** d */ {}
$foo = new ReflectionClass('foo');
echo $foo->getDocComment();
exit;
<a />;
--EXPECT--
/** d */
