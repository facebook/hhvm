<?php
class a {}
$x = (new ReflectionFunction("substr"))->getClosure();
$x->call(new a);
?>
