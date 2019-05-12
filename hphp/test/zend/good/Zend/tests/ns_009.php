<?php
class foo {
}
<<__EntryPoint>> function main() {
$x = __NAMESPACE__ . "\\foo";
echo get_class(new $x),"\n";
}
