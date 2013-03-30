<?php
namespace test\ns1;

class Exception {
}

$x = "Exception";
echo get_class(new $x),"\n";