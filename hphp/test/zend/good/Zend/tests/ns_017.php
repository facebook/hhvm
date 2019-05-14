<?php
namespace test\ns1;

function strlen($x) {
    return __FUNCTION__;
}
<<__EntryPoint>> function main() {
$x = "strlen";
echo $x("Hello"),"\n";
}
