--TEST--
Reflection API 01
--FILE--
<?php
/** a */ function /** b */ foo /** c */ (/** d */) {}
$foo = new ReflectionFunction('foo');
echo $foo->getDocComment();
exit;
<a />;
--EXPECT--
/** b */
