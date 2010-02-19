--TEST--
Reflection API 07
--FILE--
<?php
class foo {
  public $bar /** a */;
  function etc() {}
}
$foo = new ReflectionMethod('foo', 'etc');
echo $foo->getDocComment();
exit;
<a />;
--EXPECT--
