<?php
namespace test\ns1;

class Exception {
}

$x = "test\\ns1\\Exception";
echo get_class(new $x),"\n";