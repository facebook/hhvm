<?php
namespace test;

class foo {
}

$x = __NAMESPACE__ . "\\foo"; 
echo get_class(new $x),"\n";