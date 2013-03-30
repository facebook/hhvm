<?php
function foo($a,$b,$c) {
echo "foo: ".error_reporting()."\n";
}

function bar() {
echo "bar: ".error_reporting()."\n";
}

error_reporting(1);
echo "before: ".error_reporting()."\n";
@foo(1,@bar(),3);
echo "after: ".error_reporting()."\n";
?>