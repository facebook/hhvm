--TEST--
Stack Balance Fail
--FILE--
<?php
class xhp_x__y {}
$a = <x:y attr={:tag::CONSTANT} />;
function f() {}
echo 'pass';
--EXPECT--
pass
