<?php
function foo( array $x = array( 'a', 'b' ) ) {}
$r = new ReflectionParameter( 'foo', 0 );
echo $r->__toString();
?>
