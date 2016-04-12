<?php
// Ensure correct unmangling of private property names for anonymous class instances
var_dump(new class { private $foo; });
?>
