<?php
namespace Foo;
function f($a=array(Foo::bar)) {
    return $a[0];
}
<<__EntryPoint>> function main() {
echo f()."\n";
}
