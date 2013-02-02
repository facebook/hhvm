--TEST--
Logical Operator Whitespace
--FILE--
<?php
echo true xor false;
if (0) <a />;
--EXPECT--
1
