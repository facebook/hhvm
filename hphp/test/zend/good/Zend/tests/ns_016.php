<?php
namespace test\ns1;

function strlen($x) {
    return __FUNCTION__;
}
<<__EntryPoint>> function main() {
$x = "test\\ns1\\strlen";
echo $x("Hello"),"\n";
}
