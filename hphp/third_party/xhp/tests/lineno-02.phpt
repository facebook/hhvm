--TEST--
Line Numbers 02
--FILE--
<?php
function foo() {
 return <x>&#187;
   </x>;
}

function bar() {
}

$r = new ReflectionFunction('bar');
echo $r->getStartLine();
--EXPECT--
7
