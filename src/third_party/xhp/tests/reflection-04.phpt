--TEST--
Reflection API 04
--FILE--
<?php
abstract class foo {
  /** a */ abstract /** b */ public /** c */ function /** d */ bar /** e */ (/** f */);
}
$foo = new ReflectionMethod('foo', 'bar');
echo $foo->getDocComment();
exit;
<a />;
--EXPECT--
/** d */
