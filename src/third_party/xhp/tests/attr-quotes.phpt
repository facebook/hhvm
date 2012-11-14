--TEST--
Quotes in attribute
--FILE--
<?php
class xhp_a {}
$quote = '"';
echo <a b={$quote}>c</a>;
--EXPECT--
<a b="&quot;">c</a>
