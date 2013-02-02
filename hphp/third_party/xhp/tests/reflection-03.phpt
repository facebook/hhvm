--TEST--
Reflection API 03
--FILE--
<?php
class foo {
  /** a */ public /** b */ function /** c */ bar /** d */ (/** e */) {}
}
$foo = new ReflectionMethod('foo', 'bar');
echo $foo->getDocComment();
exit;
<a />;
--EXPECT--
/** c */
