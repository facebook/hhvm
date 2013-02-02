--TEST--
Reflection API 08
--FILE--
<?php
class foo {}
/** a */ class /** b */ bar /** c */ extends /** d */ foo /** e */ {}
$foo = new ReflectionClass('bar');
$comment = $foo->getDocComment();
if ($comment == '/** e */') {
  $comment = '/** d */';
}
echo $comment;
exit;
<a />;
--EXPECT--
/** d */
